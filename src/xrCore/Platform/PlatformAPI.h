 
#pragma once

#ifdef IXR_WINDOWS
#	include "Windows/PlatformAPI.h"
#elif defined(IXR_LINUX)
#	include "Linux/PlatformAPI.h"
#elif defined(IXR_BSD)
#	include "BSD/PlatformAPI.h"
#elif defined(IXR_APPLE_SERIES)
#	include "MacOS/PlatformAPI.h"
#endif

#if !defined(IXR_WINDOWS)
#include <chrono>
#include <cstdio>

// The compilers time their phases with the Win32 millisecond counter; this keeps the call
// sites identical on every non-Windows platform (macOS and Linux).
inline unsigned timeGetTime()
{
	static const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	return static_cast<unsigned>(std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - start).count());
}
#endif

// MSVC CRT spellings that the tool sources use; std has the same functions.
#include <cmath>
#include <ctime>

inline char* _ctime32(const time_t* value)
{
	return ctime(value);
}

inline int _finite(double value) { return std::isfinite(value) ? 1 : 0; }
inline int _isnan(double value) { return std::isnan(value) ? 1 : 0; }

// MSVC takes the destination first; POSIX takes the source first.
inline int localtime_s(struct tm* result, const time_t* value)
{
	return localtime_r(value, result) != nullptr ? 0 : errno;
}

// The Win32 memory helpers are plain standard library calls.
#include <cstring>
#define CopyMemory(destination, source, length) memcpy(destination, source, length)
#define ZeroMemory(destination, length) memset(destination, 0, length)

// Case-insensitive comparisons: POSIX spells them without the leading underscore.
#define _stricmp(a, b) strcasecmp(a, b)
#define _strnicmp(a, b, n) strncasecmp(a, b, n)

// Win32 helpers used by the compilers: file removal and the interlocked family. The GCC/Clang
// builtins have exactly the same semantics.
#include <cstdio>
#include <cstdint>

inline int DeleteFileA(const char* path) { return std::remove(path) == 0 ? 1 : 0; }

template<typename T>
inline T InterlockedCompareExchange(volatile T* destination, T exchange, T comparand)
{
	return __sync_val_compare_and_swap(destination, comparand, exchange);
}

// Integer literals do not deduce to the volatile type, so the Win32 overloads are explicit.
inline uint32_t InterlockedCompareExchange(volatile uint32_t* destination, uint32_t exchange, uint32_t comparand)
{
	return __sync_val_compare_and_swap(destination, comparand, exchange);
}

inline int32_t InterlockedCompareExchange(volatile int32_t* destination, int32_t exchange, int32_t comparand)
{
	return __sync_val_compare_and_swap(destination, comparand, exchange);
}

template<typename T>
inline T InterlockedIncrement(volatile T* value) { return __sync_add_and_fetch(value, 1); }

template<typename T>
inline T InterlockedDecrement(volatile T* value) { return __sync_sub_and_fetch(value, 1); }

template<typename T>
inline T InterlockedExchange(volatile T* destination, T value)
{
	return __sync_lock_test_and_set(destination, value);
}
#ifndef INFINITE
#	define INFINITE 0xFFFFFFFFu
#endif

// A unique file name in the given folder: the portable replacement for GetTempFileName.
#include <ctime>
#include <cstdio>

namespace Platform
{
	inline void MakeTempFileName(const char* folder, const char* prefix, char* out, size_t out_size)
	{
		static unsigned long serial = 0;
		std::snprintf(out, out_size, "%s%s_%08lu_%lu.tmp", folder, prefix,
			(unsigned long)std::time(nullptr), ++serial);
	}
}

// Win32 conveniences still spelled the Windows way inside the editor sources. Their
// equivalents are plain POSIX or SDL calls.
#include <SDL3/SDL.h>
#include <dlfcn.h>
#include <unistd.h>

inline HMODULE LoadLibraryA(const char* name) { return Platform::LoadLibrary(name); }
inline void* GetProcAddress(HMODULE library, const char* function) { return dlsym(library, function); }
inline BOOL FreeLibrary(HMODULE library) { Platform::FreeLibrary(library); return TRUE; }
