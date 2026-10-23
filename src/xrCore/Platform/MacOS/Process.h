#pragma once

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <spawn.h>
#include <string>
#include <vector>

extern char** environ;

namespace Platform
{
	// A child process started from a command line, with the same surface on every platform.
	class Process
	{
	public:
		static const unsigned kInfinite = 0xFFFFFFFFu;

		Process(): m_pid(-1), m_status(0), m_exited(false), m_exit_code(0) {}
		~Process() { Close(); }

		bool Start(const char* CommandLine, const char* WorkDirectory = nullptr)
		{
			Close();

			std::vector<std::string> parts = SplitCommandLine(CommandLine);
			if (parts.empty())
				return false;

			std::vector<char*> argv;
			argv.reserve(parts.size() + 1);
			for (std::string& part : parts)
				argv.push_back(const_cast<char*>(part.c_str()));
			argv.push_back(nullptr);

			pid_t pid = fork();
			if (pid < 0)
				return false;

			if (pid == 0)
			{
				if (WorkDirectory != nullptr && *WorkDirectory != 0)
					chdir(WorkDirectory);

				execvp(argv[0], argv.data());
				_exit(127);
			}

			m_pid = pid;
			m_exited = false;
			m_exit_code = 0;
			return true;
		}

		bool Valid() const { return m_pid > 0; }

		bool IsRunning() const { return m_pid > 0 && !m_exited; }

		bool GetExitCode(int& Code)
		{
			if (m_pid <= 0)
				return false;

			if (!m_exited)
			{
				int status = 0;
				const pid_t result = waitpid(m_pid, &status, WNOHANG);
				if (result == 0)
				{
					Code = kStillActive;
					return true;
				}
				if (result < 0)
					return false;

				m_exited = true;
				m_exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
			}

			Code = m_exit_code;
			return true;
		}

		bool Wait(unsigned TimeoutMs, int* ExitCode = nullptr)
		{
			if (m_pid <= 0)
				return false;

			if (!m_exited)
			{
				if (TimeoutMs == kInfinite)
				{
					int status = 0;
					if (waitpid(m_pid, &status, 0) < 0)
						return false;
					m_exited = true;
					m_exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
				}
				else
				{
					// Poll: the editors only ever wait for short-lived tools.
					const unsigned step = 10;
					for (unsigned elapsed = 0; elapsed <= TimeoutMs; elapsed += step)
					{
						int code = 0;
						if (!GetExitCode(code))
							return false;
						if (code != kStillActive)
							break;
						usleep(step * 1000);
					}
				}
			}

			if (ExitCode != nullptr)
				*ExitCode = m_exit_code;
			return m_exited;
		}

		void Close()
		{
			// A finished process is reaped by GetExitCode/Wait; a running one is detached so it
			// is not killed when the editor closes the handle.
			m_pid = -1;
			m_exited = false;
			m_exit_code = 0;
		}

		static const int kStillActive = 259;

	private:
		Process(const Process&) = delete;
		void operator=(const Process&) = delete;

		static std::vector<std::string> SplitCommandLine(const char* command_line)
		{
			std::vector<std::string> result;

			if (command_line == nullptr)
				return result;

			const char* p = command_line;
			while (*p != 0)
			{
				while (*p == ' ' || *p == '\t')
					++p;
				if (*p == 0)
					break;

				std::string part;
				if (*p == '"')
				{
					++p;
					while (*p != 0 && *p != '"')
						part += *p++;
					if (*p == '"')
						++p;
				}
				else
				{
					while (*p != 0 && *p != ' ' && *p != '\t')
						part += *p++;
				}
				result.push_back(part);
			}

			return result;
		}

		pid_t m_pid;
		int m_status;
		bool m_exited;
		int m_exit_code;
	};
}
