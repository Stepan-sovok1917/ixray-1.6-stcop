#pragma once

// The taskbar progress indicator exists only on Windows; other platforms get the same
// interface with empty bodies so the callers stay unchanged.
#ifdef IXR_WINDOWS

#include <Windows.h>
#include <ShObjIdl.h>
#include <string>
#include "../Resources/resource.h" // >>> IDI_OVERLAY_*

// >>> UX-PROGRESS
// Идентификаторы оверлей-иконок, которые могут быть показаны на иконке
// приложения в панели задач.
enum class TaskbarOverlay : int
{
	None = 0, // снять оверлей
	XrLC,	  // стадия xrLC
	XrAI,	  // стадия xrAI
	XrDO,	  // стадия xrDO
	Success,  // успешное завершение
	Error	  // ошибка
};
// <<< UX-PROGRESS


class CTaskbarProgress
{
public:
	static CTaskbarProgress& Instance();

	bool Initialize(HWND hwnd);
	void Release();

	void SetState(TBPFLAG state);
	void SetProgress(ULONGLONG completed, ULONGLONG total);
	void SetMarquee(bool enable);
	void Reset();

	// Подсказка над иконкой в панели задач.
	void SetTooltip(const std::wstring& text);

	// >>> UX-PROGRESS: оверлей-иконка
	// Устанавливает оверлей. Если hIcon == nullptr, оверлей снимается.
	void SetOverlayIcon(HICON hIcon, const std::wstring& description);
	void SetOverlayIcon(TaskbarOverlay overlay, const std::wstring& description);

	// Загрузить иконки из ресурсов приложения. Возвращает false, если
	// хотя бы одна иконка не загружена.
	bool LoadOverlayIcons();

	// Освободить загруженные иконки.
	void ReleaseOverlayIcons();

	bool IsInitialized() const { return m_pTaskbar != nullptr && m_hwnd != nullptr; }
	HWND GetHwnd() const { return m_hwnd; }
	// <<< UX-PROGRESS

private:
	CTaskbarProgress() = default;
	~CTaskbarProgress();
	CTaskbarProgress(const CTaskbarProgress&) = delete;
	CTaskbarProgress& operator=(const CTaskbarProgress&) = delete;

	ITaskbarList3* m_pTaskbar = nullptr;
	HWND m_hwnd = nullptr;
	bool m_needCoUninit = false;

	// >>> UX-PROGRESS
	HICON m_hIconXrLC = nullptr;
	HICON m_hIconXrAI = nullptr;
	HICON m_hIconXrDO = nullptr;
	HICON m_hIconSuccess = nullptr;
	HICON m_hIconError = nullptr;

	HICON GetIconForOverlay(TaskbarOverlay overlay) const;
	// <<< UX-PROGRESS
};
#else
enum : int // taskbar progress states, mirroring the Windows values
{
	TBPF_NOPROGRESS = 0x0,
	TBPF_INDETERMINATE = 0x1,
	TBPF_NORMAL = 0x2,
	TBPF_ERROR = 0x4,
	TBPF_PAUSED = 0x8
};

enum class TaskbarOverlay : int
{
	None = 0,
	XrLC,
	XrAI,
	XrDO,
	Success,
	Error
};

class CTaskbarProgress
{
public:
	bool Initialize(void*) { return false; }
	void Release() {}
	void SetState(int) {}
	void SetProgress(unsigned long long, unsigned long long) {}
	void SetMarquee(bool) {}
	void Reset() {}
	void SetTooltip(const std::wstring&) {}
	void SetOverlayIcon(void*, const std::wstring&) {}
	void SetOverlayIcon(TaskbarOverlay, const std::wstring&) {}
	bool IsInitialized() const { return false; }
	static CTaskbarProgress& Instance() { static CTaskbarProgress instance; return instance; }
};

#endif
