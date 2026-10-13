// PlatformWindows.cpp

#ifdef _WIN32
#include "Platform.h"
#include <Windows.h>

namespace Platform {

    // --- Dynamic Library ---
    DynamicLibrary::Handle DynamicLibrary::Load(const fs::path& path) {
        return static_cast(LoadLibraryW(path.wstring().c_str()));
    }

    void DynamicLibrary::Unload(Handle handle) {
        if (handle) {
            FreeLibrary(static_cast(handle));
        }
    }

    void* DynamicLibrary::GetSymbol(Handle handle, const std::string& symbol) {
        if (!handle) return nullptr;
        return reinterpret_cast(GetProcAddress(static_cast(handle), symbol.c_str()));
    }

    std::string DynamicLibrary::GetLastErrorString() {
        DWORD err = GetLastError();
        if (err == 0) return "";

        LPSTR buf = nullptr;
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, nullptr
        );

        std::string message(buf, size);
        LocalFree(buf);
        return message;
    }

    // --- Paths ---
    fs::path Paths::GetProcessDirectory() {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        return fs::path(path).parent_path();
    }

    fs::path Paths::GetRuntimeDirectory() {
        HMODULE hModule = nullptr;
        GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast(&Paths::GetRuntimeDirectory),
            &hModule
        );

        wchar_t path[MAX_PATH];
        GetModuleFileNameW(hModule, path, MAX_PATH);
        return fs::path(path).parent_path();
    }

    fs::path Paths::GetPluginsDirectory() {
        fs::path pluginPath = GetProcessDirectory() / ".." / ".." / "Plugins";
		pluginPath = fs::weakly_canonical(pluginPath);
        if (!fs::exists(pluginPath)) {
            fs::create_directories(pluginPath);
        }
        return pluginPath;
    }

    // --- Process ---
    void* Process::GetModuleBaseAddress(const char* moduleName) {
        if (!moduleName) {
            return static_cast(GetModuleHandleA(nullptr));
        }
        return static_cast(GetModuleHandleA(moduleName));
    }

    unsigned long Process::GetCurrentProcessId() {
        return ::GetCurrentProcessId();
    }

    unsigned long Process::GetCurrentThreadId() {
        return ::GetCurrentThreadId();
    }

    void Process::SetThreadName(const std::string& name) {
        // Windows 10 1607+ thread naming API
        typedef HRESULT(WINAPI* SetThreadDescriptionFunc)(HANDLE, PCWSTR);
        auto setThreadDesc = (SetThreadDescriptionFunc)GetProcAddress(GetModuleHandleA("kernel32.dll"), "SetThreadDescription");
        if (setThreadDesc) {
            std::wstring wname(name.begin(), name.end());
            setThreadDesc(GetCurrentThread(), wname.c_str());
        }
    }

} // namespace EMR::Platform
#endif // _WIN32