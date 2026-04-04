#pragma once
#include "pch.h"
#include <string>
#include <memory>

// ref: https://github.com/M0rtale/Universal-WndProc-Hook/

namespace HK {
	// Window Procedure for readability, Microsoft uses "WndProc"
	class WindowProcedure {
	// statics
	private:
		static WNDPROC targetWndProc;
		// hook wrapper will be called each time WndProc is called in target
		static auto hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT __stdcall;
	public:
		// wide string, UTF-16 Windows default
		static auto createHook(const std::wstring& targetWindowName) -> std::unique_ptr<WindowProcedure>;
	// object
	private:
		WindowProcedure(HWND hWindow);
		HWND _hWindow;
	public:
		auto restore() const -> void;
	};
};

