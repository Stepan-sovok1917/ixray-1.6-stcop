#include "../../xrCore/xrCore.h"
#include "../../xrCore/FormatParsers/json/JsonSerialize.h"
#include "Common/cl_log.h"
#include "UI/CompilersUI.h"
#include "UI/ToastNotify.h"

// The level builder runs as a console program outside Windows. The build phases and the
// settings live in xrFrontend.cpp next to the window front-end, and the modes are selected
// with the same command line switches this tool has always accepted:
//   -f <level>   compile one level, -no_separator_check / -draft / -verify belong to xrAI.

extern CompilersMode gCompilerMode;
extern CJsonSerializer* Serializer;

void LoadCompilerSettings();
void SaveCompilerCfg();
void LoadSelectedLevels();
void Startup(LPSTR lpCmdLine);

void SEFactoryEntry();
void SEFactoryDestroy();

int main(int argc, char** argv)
{
	Debug._initialize(false);

	// The compiler phases parse one command line string, so the arguments are joined the same
	// way the window build joined them.
	xr_string CommandLine;
	for (int arg = 1; arg < argc; ++arg)
	{
		if (arg > 1)
		{
			CommandLine += " ";
		}

		const bool HasSpace = strchr(argv[arg], ' ') != nullptr;
		if (HasSpace)
		{
			CommandLine += "\"";
		}
		CommandLine += argv[arg];
		if (HasSpace)
		{
			CommandLine += "\"";
		}
	}

	const char* fsgame_ltx_name = "-fsltx ";
	string_path fsgame = "";

	if (strstr(CommandLine.c_str(), fsgame_ltx_name))
	{
		const int sz = xr_strlen(fsgame_ltx_name);
		sscanf(strstr(CommandLine.c_str(), fsgame_ltx_name) + sz, "%519[^ ] ", fsgame);
	}

	// The SDK description of the file system is the one the builder is documented with.
	Core._initialize("IX-Ray Compilers", nullptr, true, fsgame[0] ? fsgame : "fs.ltx");

	SEFactoryEntry();

	// No windows in a console run: the notifications go to the log.
	CToastNotify::Instance().SetSilent(true);

	Serializer = new CJsonSerializer("xrlevelbuilder.json");
	LoadCompilerSettings();

	// No window here: the phases run right away and report on the console and in the log.
	LoadSelectedLevels();
	Startup((LPSTR)CommandLine.c_str());

	SaveCompilerCfg();
	SEFactoryDestroy();

	xr_delete(Serializer);

	return 0;
}
