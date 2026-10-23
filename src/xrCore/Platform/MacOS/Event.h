#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace Platform
{
	// A manual- or auto-reset event, the portable counterpart of the Win32 event object.
	class Event
	{
	public:
		static const unsigned kInfinite = 0xFFFFFFFFu;

		explicit Event(bool manual_reset = false, bool signalled = false)
			: m_manual_reset(manual_reset), m_signalled(signalled) {}

		void Set()
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_signalled = true;
			}

			if (m_manual_reset)
				m_condition.notify_all();
			else
				m_condition.notify_one();
		}

		void Reset()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_signalled = false;
		}

		bool Wait(unsigned timeout_ms = kInfinite)
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			const auto ready = [this] { return m_signalled; };

			if (timeout_ms == kInfinite)
				m_condition.wait(lock, ready);
			else if (!m_condition.wait_for(lock, std::chrono::milliseconds(timeout_ms), ready))
				return false;

			if (!m_manual_reset)
				m_signalled = false;

			return true;
		}

	private:
		Event(const Event&) = delete;
		void operator=(const Event&) = delete;

		std::mutex m_mutex;
		std::condition_variable m_condition;
		bool m_manual_reset;
		bool m_signalled;
	};
}
