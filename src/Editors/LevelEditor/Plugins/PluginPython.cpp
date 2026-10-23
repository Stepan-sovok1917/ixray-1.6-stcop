#include "stdafx.h"
#include "PluginPython.h"

#include <fstream>

bool CPluginPython::IsPythonInstalled() const
{
	int exitCode = std::system("python --version >nul 2>&1");
	if (exitCode == 0) 
		return true;

	exitCode = std::system("python3 --version >nul 2>&1");
	return exitCode == 0;
}

xr_string CPluginPython::RunCommand(const xr_string& command)
{
	// popen runs the line through the shell and hands back its combined output, which is what
	// the Win32 pipe dance did.
	string_path Root;
	FS.update_path(Root, "$fs_root$", "");

	xr_string CommandLine = "cd \"";
	CommandLine += Root;
	CommandLine += "\" && ";
	CommandLine += command;
	CommandLine += " 2>&1";

	FILE* Pipe = popen(CommandLine.c_str(), "r");
	if (Pipe == nullptr)
	{
		Msg("! Error starting process...");
		return "";
	}

	string128 buffer;
	xr_string output;

	while (fgets(buffer, sizeof(buffer), Pipe) != nullptr)
	{
		output += buffer;
	}

	pclose(Pipe);
	return Platform::ANSI_TO_UTF8(output);
}
CPluginPython::CPluginPython()
{
	Type = EPluginType::Python;
}

void CPluginPython::Run()
{
	if (!IsPythonInstalled())
	{
		Msg("! Not found Python SDK!");
		return;
	}

	xr_string Command = "python \"";
	Command += Path;
	Command += "\"";

	if (IsSimple())
	{
		size_t ExtPos = Scene->full_name.find(".level");

		if (ExtPos == xr_string::npos)
		{
			Msg("! Level is empty...");
			return;
		}
		xr_string LevelDir = Scene->full_name.substr(0, ExtPos) + "\\";
		Command += " -level=\"";
		Command += Scene->full_name;
		Command += "\"";
	}
	else
	{
		for (auto& [Arg, Value] : InputArgsValues)
		{
			Command += " -";
			Command += Arg;
			Command += "=\"";
			Command += Value;
			Command += "\"";
		}
	}

	Msg(RunCommand(Command).c_str());
}

xr_string CPluginPython::ReadDesc() const
{
	std::ifstream file(Path.data());

	xr_string line;
	xr_string result = "Not found description!";

	const std::string_view prefix = "# desc: ";
	const std::string_view prefix_input = "# input: ";

	while (std::getline(file, line))
	{
		if (line.rfind(prefix, 0) == 0)
		{
			size_t pos = line.find(':');
			if (pos != xr_string::npos && pos + 1 < line.size())
			{
				result = line.substr(pos + 1);
			}
		}
		else if (line.rfind(prefix_input, 0) == 0)
		{
			std::regex pattern(R"(\[([^,]+), ([^\]]+)\])");

			std::smatch match;
			xr_string::const_iterator searchStart(line.cbegin());

			while (std::regex_search(searchStart, line.cend(), match, pattern))
			{
				InputArgsName[match[1].str().c_str()] = match[2].str().c_str();
				searchStart = match.suffix().first;
			}
		}
	}

	return result;
}