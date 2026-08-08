#pragma once

#include "common.h"
#include <cstddef>
#include <expected>
#include <span>
#include <utility>
#include <Windows.h>
#include <TlHelp32.h>

class Process {
public:
	Process() = default;

	Process(const wchar_t* procName, uintptr_t modBase)
		: _pid{ getPIDByName(procName) },
		_modBase{ modBase },
		_handle{ OpenProcess(PROCESS_ALL_ACCESS, FALSE, _pid) }
	{}

	~Process() { reset(); }

	Process(const Process&)            = delete;
	Process& operator=(const Process&) = delete;

	Process(Process&& other) noexcept { swap(other); }
	Process& operator=(Process&& other) noexcept
	{
		if (this != &other) { reset(); swap(other); }
		return *this;
	}

	DWORD     pid()     const { return _pid; }
	uintptr_t modBase() const { return _modBase; }
	HANDLE    handle()  const { return _handle; }

private:
	void reset() noexcept
	{
		if (_handle) { CloseHandle(_handle); _handle = nullptr; }
	}

	void swap(Process& other) noexcept
	{
		std::swap(_pid, other._pid);
		std::swap(_modBase, other._modBase);
		std::swap(_handle, other._handle);
	}

	static DWORD getPIDByName(const wchar_t* procName)
	{
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnap == INVALID_HANDLE_VALUE)
			return 0;

		DWORD procId = 0;
		PROCESSENTRY32 procEntry{};
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, procName))
				{
					procId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}

		CloseHandle(hSnap);
		return procId;
	}

	DWORD     _pid{ 0 };
	uintptr_t _modBase{ 0 };
	HANDLE    _handle{ nullptr };
};

namespace memory {

	inline bool read(HANDLE process, uintptr_t addr, void* dst, std::size_t size)
	{
		SIZE_T bytesRead = 0;
		return (ReadProcessMemory(process, reinterpret_cast<LPCVOID>(addr), dst, size, &bytesRead))
			&& (bytesRead == size);
	}

	template <class T>
	bool read(HANDLE process, uintptr_t addr, std::span<T> dst)
	{
		return read(process, addr, dst.data(), dst.size_bytes());
	}

	template <class T>
	std::expected<T, DWORD> readValue(HANDLE process, uintptr_t addr)
	{
		T value{};
		if (!read(process, addr, &value, sizeof(value)))
			return std::unexpected(GetLastError());

		return value;
	}
}
