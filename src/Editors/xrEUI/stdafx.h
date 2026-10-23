#pragma once

#ifdef XREUI_EXPORTS
#define XREUI_API __declspec(dllexport)
#else
#define XREUI_API __declspec(dllimport)
#endif

#define IM_TEXTURE_RELEASE(tex) ((IUnknown*)(tex))->Release()

#include "../../xrCore/xrCore.h"

enum class EDragDropType : u8
{
	Folder,
	File,
	Viewport,
	RandomAppend,
	Details,
	Logic,

	None
};

#include "EditorWnd.h"
#include "XrUIManager.h"

#define IMGUI_IMPL_API
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"

namespace ImGui
{
	XREUI_API bool HyperLink(const char* label, const char* url, bool underlineWhenHoveredOnly = false);
}

// The Win32 compatibility headers used by the editors bring a C-style NULL; the editor code is
// C++ and compares pointers, so keep the C++ spelling everywhere.
#ifdef __cplusplus
#	undef NULL
#	define NULL nullptr
#endif

// The cursor helpers are spelled the Windows way in the editor sources; this is their SDL version.
#include <SDL3/SDL.h>

inline int GetCursorPos(void* point)
{
	float x = 0.0f, y = 0.0f;
	SDL_GetGlobalMouseState(&x, &y);
	// POINT is { int32_t x; int32_t y; } in the compatibility headers; kept type-agnostic here
	// because the shared header is parsed before those definitions exist.
	struct LocalPoint { int32_t x; int32_t y; };
	LocalPoint* cursor = static_cast<LocalPoint*>(point);
	cursor->x = (int32_t)x;
	cursor->y = (int32_t)y;
	return 1;
}

inline int SetCursorPos(int x, int y)
{
	SDL_WarpMouseGlobal((float)x, (float)y);
	return 1;
}
