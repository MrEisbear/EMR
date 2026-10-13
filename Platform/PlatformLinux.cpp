// PlatformLinux.cpp

#ifdef __linux__

#include "Platform.h"

#include <dlfcn.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace EMR::Platform {

    // --- Dynamic Library ---

    DynamicLibrary::Handle DynamicLibrary::Load(const fs::path& path) {
        return dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    }

    void DynamicLibrary::Unload(Handle handle) {
        if (handle) {
            dlclose(handle);
        }
    }

    void* DynamicLibrary::GetSymbol(Handle handle, const std::string& symbol) {
        if (!handle) {
            return nullptr;
        }

        return dlsym(handle, symbol.c_str());
    }

    std::string DynamicLibrary::GetLastErrorString() {
        const char* error = dlerror();
        return error ? error : "";
    }


    // --- Paths ---

    fs::path Paths::GetProcessDirectory() {
        char path[4096];

        ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);

        if (length < 0) {
            return fs::current_path();
        }

        path[length] = '\0';

        return fs::path(path).parent_path();
    }

    fs::path Paths::GetRuntimeDirectory() {
        Dl_info info{};

        if (dladdr(
            reinterpret_cast<void*>(&Paths::GetRuntimeDirectory),
            &info
        ) && info.dli_fname) {
            return fs::path(info.dli_fname).parent_path();
        }

        return GetProcessDirectory();
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
        void* handle;

        if (moduleName) {
            handle = dlopen(moduleName, RTLD_LAZY | RTLD_NOLOAD);
        }
        else {
            handle = dlopen(nullptr, RTLD_LAZY);
        }

        if (!handle) {
            return nullptr;
        }

        Dl_info info{};

        // dlsym gives us an address belonging to the module.
        // dladdr can then resolve the module containing that address.
        void* address = dlsym(handle, moduleName ? "main" : "main");

        if (!address) {
            dlclose(handle);
            return nullptr;
        }

        if (dladdr(address, &info)) {
            dlclose(handle);
            return info.dli_fbase;
        }

        dlclose(handle);
        return nullptr;
    }

    unsigned long Process::GetCurrentProcessId() {
        return static_cast<unsigned long>(getpid());
    }

    unsigned long Process::GetCurrentThreadId() {
        return static_cast<unsigned long>(syscall(SYS_gettid));
    }

    void Process::SetThreadName(const std::string& name) {
        pthread_setname_np(
            pthread_self(),
            name.substr(0, 15).c_str()
        );
    }

}

#endif