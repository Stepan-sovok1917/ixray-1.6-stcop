#pragma once

namespace Platform
{
	// A child process started from a command line, with the same surface on every platform.
	class Process
	{
	public:
		static const unsigned kInfinite = 0xFFFFFFFFu;
		static const int kStillActive = STILL_ACTIVE;

		Process(): m_process(nullptr), m_thread(nullptr) {}
		~Process() { Close(); }

		bool Start(const char* CommandLine, const char* WorkDirectory = nullptr)
		{
			Close();

			STARTUPINFOA startup = {};
			startup.cb = sizeof(startup);

			char* mutable_line = _strdup(CommandLine);
			if (mutable_line == nullptr)
				return false;

			const BOOL started = CreateProcessA(nullptr, mutable_line, nullptr, nullptr, TRUE,
				CREATE_NO_WINDOW, nullptr, WorkDirectory, &startup, &m_info);

			free(mutable_line);

			if (!started)
				return false;

			m_process = m_info.hProcess;
			m_thread = m_info.hThread;
			return true;
		}

		bool Valid() const { return m_process != nullptr; }

		bool IsRunning() const
		{
			int code = 0;
			return const_cast<Process*>(this)->GetExitCode(code) && code == kStillActive;
		}

		bool GetExitCode(int& Code)
		{
			if (m_process == nullptr)
				return false;

			DWORD code = 0;
			if (!::GetExitCodeProcess(m_process, &code))
				return false;

			Code = static_cast<int>(code);
			return true;
		}

		bool Wait(unsigned TimeoutMs, int* ExitCode = nullptr)
		{
			if (m_process == nullptr)
				return false;

			const DWORD timeout = (TimeoutMs == kInfinite) ? INFINITE : TimeoutMs;
			if (::WaitForSingleObject(m_process, timeout) != WAIT_OBJECT_0)
				return false;

			if (ExitCode != nullptr)
				return GetExitCode(*ExitCode);
			return true;
		}

		void Terminate()
		{
			if (m_process != nullptr)
				::TerminateProcess(m_process, 1);
		}

		void Close()
		{
			if (m_thread != nullptr)
			{
				::CloseHandle(m_thread);
				m_thread = nullptr;
			}
			if (m_process != nullptr)
			{
				::CloseHandle(m_process);
				m_process = nullptr;
			}
		}

	private:
		Process(const Process&) = delete;
		void operator=(const Process&) = delete;

		PROCESS_INFORMATION m_info = {};
		HANDLE m_process;
		HANDLE m_thread;
	};
}
