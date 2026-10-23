#pragma once

namespace Platform
{
	// A manual- or auto-reset event, the portable counterpart of the Win32 event object.
	class Event
	{
	public:
		static const unsigned kInfinite = 0xFFFFFFFFu;

		explicit Event(bool manual_reset = false, bool signalled = false)
		{
			m_event = ::CreateEventA(nullptr, manual_reset ? TRUE : FALSE,
				signalled ? TRUE : FALSE, nullptr);
		}

		~Event()
		{
			if (m_event != nullptr)
				::CloseHandle(m_event);
		}

		void Set() { ::SetEvent(m_event); }
		void Reset() { ::ResetEvent(m_event); }

		bool Wait(unsigned timeout_ms = kInfinite)
		{
			const DWORD timeout = (timeout_ms == kInfinite) ? INFINITE : timeout_ms;
			return ::WaitForSingleObject(m_event, timeout) == WAIT_OBJECT_0;
		}

	private:
		Event(const Event&) = delete;
		void operator=(const Event&) = delete;

		HANDLE m_event = nullptr;
	};
}
