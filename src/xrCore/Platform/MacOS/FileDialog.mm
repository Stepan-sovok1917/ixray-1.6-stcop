#include <string>

#ifdef __APPLE__

#import <Cocoa/Cocoa.h>

// The system panels run their own modal loop, so the editor does not have to pump events while
// the user browses the disk. Patterns come in the engine form ("*.level;*.tmp").
bool MacFileDialog(bool bSave, const char* DefaultDirectory, const char* Patterns, std::string& Out)
{
	@autoreleasepool
	{
		NSSavePanel* Panel = bSave ? [NSSavePanel savePanel] : (NSSavePanel*)[NSOpenPanel openPanel];

		if (!bSave)
		{
			NSOpenPanel* OpenPanel = (NSOpenPanel*)Panel;
			[OpenPanel setCanChooseFiles:YES];
			[OpenPanel setCanChooseDirectories:NO];
			[OpenPanel setAllowsMultipleSelection:NO];
		}

		NSMutableArray<NSString*>* Types = [NSMutableArray array];
		const std::string PatternList = Patterns ? Patterns : "";
		size_t Start = 0;

		while (Start <= PatternList.size())
		{
			const size_t Separator = PatternList.find(';', Start);
			const std::string Item = PatternList.substr(Start, Separator == std::string::npos ? std::string::npos : Separator - Start);
			const size_t Dot = Item.rfind('.');

			if (Dot != std::string::npos && Dot + 1 < Item.size())
			{
				NSString* Extension = [NSString stringWithUTF8String:Item.substr(Dot + 1).c_str()];

				if (Extension)
				{
					[Types addObject:Extension];
				}
			}

			if (Separator == std::string::npos)
			{
				break;
			}

			Start = Separator + 1;
		}

		if ([Types count] > 0)
		{
			[Panel setAllowedFileTypes:Types];
		}

		if (DefaultDirectory && DefaultDirectory[0])
		{
			NSString* Directory = [NSString stringWithUTF8String:DefaultDirectory];

			if (Directory)
			{
				[Panel setDirectoryURL:[NSURL fileURLWithPath:Directory isDirectory:YES]];
			}
		}

		if ([Panel runModal] != NSModalResponseOK)
		{
			return false;
		}

		NSURL* Selected = [Panel URL];

		if (Selected == nil || [Selected path] == nil)
		{
			return false;
		}

		Out = [[Selected path] UTF8String];
		return !Out.empty();
	}
}

#else

bool MacFileDialog(bool, const char*, const char*, std::string&)
{
	return false;
}

#endif
