#ifndef PROC_H
#define PROC_H

#include <cstddef>
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcId(const wchar_t* procName);

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);

namespace Memory {
	std::vector<std::byte> ReadBytes(HANDLE hProcess, uintptr_t ptr, SIZE_T length);
}

#endif