#include "EisbearsModRuntime.h"

#ifdef _WIN32 // Windows-specific code
#include <windows.h>
DWORD WINAPI InitRuntimeThreadWindows(LPVOID lpParam) {
	InitRuntime();
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		
		HANDLE InitThread = CreateThread(nullptr, 0, InitRuntimeThreadWindows, nullptr, 0, nullptr);
		if (InitThread == nullptr) {
			DWORD dwError = GetLastError();
			std::wstring errorMessage = L"Failed to create initialization thread. Error code: " + std::to_wstring(dwError) + L"\n";
			LPCWSTR errorMessageCStr = errorMessage.c_str();
			OutputDebugStringW(errorMessageCStr);
		}
		else {
			CloseHandle(InitThread);
		}
	}
	return TRUE;
}
#endif // _WIN32
#ifdef __linux__ // Linux-specific code
#include <pthread.h>
#include <iostream>
#include <cstring>

void* InitRuntimeThreadLinux(void* param) {
	InitRuntime();
	return nullptr;
}
__attribute__((constructor))
void LinuxOnLoad() {
	pthread_t thread;
	// Spawn background thread and exit initialization handler
	int ret = pthread_create(&thread, nullptr, InitRuntimeThreadLinux, nullptr);
	if (ret != 0) {
		std::cerr << "Failed to create initialization thread: " << strerror(ret) << std::endl;
	}
	else {
		pthread_detach(thread);
	}
}
#endif // __linux__

namespace Platform {

}