#pragma once
#include <dlfcn.h>
#include <string>
#include <alloca.h>
#include <malloc/malloc.h>
#include <mach-o/dyld.h>

inline size_t malloc_usable_size(void* p)
{
    return malloc_size(p);
}

#define _msize malloc_usable_size
#define _expand(p, sz) sz <= _msize(p)
#define _alloca alloca

namespace Platform
{
    inline HMODULE LoadLibrary(const char* Name)
    {
        // Callers pass names with or without the "lib" prefix and the ".dylib" suffix, and
        // macOS never searches the directory of the executable on its own, so the name is
        // normalised and every reasonable location is tried.
        std::string Base = Name;
        if (Base.size() > 6 && Base.compare(Base.size() - 6, 6, ".dylib") == 0)
        {
            Base.resize(Base.size() - 6);
        }
        if (Base.rfind("lib", 0) == 0)
        {
            Base = Base.substr(3);
        }

        char ExecPath[PATH_MAX] = {};
        uint32_t ExecPathSize = sizeof(ExecPath);
        _NSGetExecutablePath(ExecPath, &ExecPathSize);
        if (char* Slash = strrchr(ExecPath, '/'))
        {
            *Slash = '\0';
        }

        const std::string Candidates[] =
        {
            std::string(Name),
            std::string(ExecPath) + "/" + Name,
            std::string(ExecPath) + "/lib" + Base + ".dylib",
            std::string(ExecPath) + "/" + Base + ".dylib",
            "lib" + Base + ".dylib",
            Base + ".dylib",
        };

        for (const std::string& Candidate : Candidates)
        {
            if (void* Module = dlopen(Candidate.c_str(), RTLD_NOW | RTLD_GLOBAL))
            {
                return Module;
            }
        }

        return nullptr;
    }

    inline void* GetAddress(HMODULE Library, const char* Function)
    {
        return dlsym(Library, Function);
    }

    inline void FreeLibrary(HMODULE Library)
    {
        dlclose(Library);
    }
}