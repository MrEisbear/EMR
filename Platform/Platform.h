#pragma once

#include <filesystem>
#include <string>

namespace Platform {

    namespace fs = std::filesystem;

    class DynamicLibrary {
    public:
        using Handle = void*;

        /// <summary>
        /// Loads a dynamic library from the specified path.
        /// </summary>
        /// <param name="path">Path to the dynamic library.</param>
        /// <returns>
        /// A handle to the loaded library, or nullptr if loading failed.
        /// </returns>
        static Handle Load(const fs::path& path);

        /// <summary>
        /// Unloads a previously loaded dynamic library.
        /// </summary>
        /// <param name="handle">Handle returned by Load().</param>
        static void Unload(Handle handle);

        /// <summary>
        /// Retrieves the address of a symbol exported by a dynamic library.
        /// </summary>
        /// <param name="handle">Handle returned by Load().</param>
        /// <param name="symbol">Name of the exported symbol.</param>
        /// <returns>
        /// Address of the symbol, or nullptr if the symbol was not found.
        /// </returns>
        static void* GetSymbol(
            Handle handle,
            const std::string& symbol
        );

        /// <summary>
        /// Returns the most recent dynamic library error.
        /// </summary>
        /// <returns>
        /// A human-readable error message, or an empty string if no error occurred.
        /// </returns>
        static std::string GetLastErrorString();
    };


    // --- Paths ---

    class Paths {
    public:

        /// <summary>
        /// Gets the directory containing the runtime library.
        /// </summary>
        /// <returns>
        /// Directory containing the EMR runtime library.
        /// </returns>
        static fs::path GetRuntimeDirectory();

        /// <summary>
        /// Gets the directory containing the host process.
        /// </summary>
        /// <returns>
        /// Directory containing the host process executable.
        /// </returns>
        static fs::path GetProcessDirectory();

        /// <summary>
        /// Gets the runtime Plugins directory.
        /// </summary>
        /// <returns>
        /// Path to the Plugins directory.
        /// The directory is created if it does not already exist.
        /// </returns>
        static fs::path GetPluginsDirectory();
    };


    // --- Process ---

    class Process {
    public:

        /// <summary>
        /// Gets the base address of a loaded module.
        /// </summary>
        /// <param name="moduleName">
        /// Name of the module to find. Pass nullptr to get the base
        /// address of the main executable.
        /// </param>
        /// <returns>
        /// Base address of the module, or nullptr if the module was not found.
        /// </returns>
        static void* GetModuleBaseAddress(
            const char* moduleName = nullptr
        );

        /// <summary>
        /// Gets the identifier of the current process.
        /// </summary>
        /// <returns>
        /// Identifier of the current process.
        /// </returns>
        static unsigned long GetCurrentProcessId();

        /// <summary>
        /// Gets the identifier of the current thread.
        /// </summary>
        /// <returns>
        /// Identifier of the current thread.
        /// </returns>
        static unsigned long GetCurrentThreadId();

        /// <summary>
        /// Sets the name of the current thread.
        /// </summary>
        /// <param name="name">Name to assign to the current thread.</param>
        static void SetThreadName(const std::string& name);
    };

}