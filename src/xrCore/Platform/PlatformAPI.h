 
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
