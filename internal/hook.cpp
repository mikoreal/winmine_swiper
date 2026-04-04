#include "pch.h"
#include "hook.h"

namespace HK {

	WNDPROC WindowProcedure::targetWndProc = nullptr;

	WindowProcedure::WindowProcedure(HWND hWindow) : _hWindow{ hWindow }
	{}

	auto WindowProcedure::restore() const -> void
	{
		if (_hWindow && WindowProcedure::targetWndProc) {
			SetWindowLongPtrW(
				_hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcedure::targetWndProc)
			);
		}
	}

	auto WindowProcedure::hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT __stdcall
	{

		// Call the original Function, not to disrupt target flow
		return CallWindowProcW(WindowProcedure::targetWndProc, hWnd, uMsg, wParam, lParam);
	}

	auto WindowProcedure::createHook(const std::wstring& targetWindowName) -> std::unique_ptr<WindowProcedure>
	{
		auto hWindow = FindWindow(nullptr, targetWindowName.c_str());
		if (not hWindow) {
			return nullptr;
		}

		WindowProcedure::targetWndProc = reinterpret_cast<WNDPROC>(
			SetWindowLongPtr(hWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcedure::hkWndProc))
		);
		if (not WindowProcedure::targetWndProc) {
			return nullptr; // hook failed
		}

		return std::unique_ptr<WindowProcedure>(new WindowProcedure{hWindow});
	}


}