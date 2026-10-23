#pragma once

class CToastNotify
{
public:
	static CToastNotify& Instance();

	// A console run reports in the log only: a modal box would wait for a click forever.
	void SetSilent(bool bSilent) { Silent = bSilent; }

	bool Initialize();
	void Shutdown();
	bool IsInitialized() const { return Initialized; }

	const std::wstring& GetAumi() const { return Aumi; }

	void Show(const std::wstring& title, const std::wstring& message);
	void ShowInfo(const std::wstring& title, const std::wstring& message);
	void ShowSuccess(const std::wstring& title, const std::wstring& message);
	void ShowError(const std::wstring& title, const std::wstring& message);

private:
	CToastNotify() = default;
	~CToastNotify();
	CToastNotify(const CToastNotify&) = delete;
	CToastNotify& operator=(const CToastNotify&) = delete;

	bool ShowFallback(const std::wstring& title, const std::wstring& message);

#ifdef IXR_WINDOWS
	bool ShowWinRT(const std::wstring& title, const std::wstring& message, bool errorStyle);
#endif

	bool Initialized = false;
	bool Silent = false;
	std::wstring Aumi;
};