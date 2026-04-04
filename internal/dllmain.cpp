// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "common.h"
#include "hook.h"

DWORD WINAPI MineswiperThread(LPVOID lpParam)
{
	auto inputHook = HK::WindowProcedure::createHook(game::windowName);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		
		// we don't need notifications about other threads spawning and dying in winmine
		DisableThreadLibraryCalls(hModule); // https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-disablethreadlibrarycalls
		if (const auto hThread = CreateThread(nullptr, size_t{ 0 }, MineswiperThread, hModule, DWORD{ 0 }, nullptr)) {
			CloseHandle(hThread);
		}


	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

