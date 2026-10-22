 
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
