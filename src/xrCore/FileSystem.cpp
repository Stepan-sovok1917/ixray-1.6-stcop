//----------------------------------------------------
// file: FileSystem.cpp
//----------------------------------------------------

#include "stdafx.h"


EFS_Utils* xr_EFS = nullptr;
//----------------------------------------------------
xr_string	EFS_Utils::ExtractFileName(const char* src)
{
	string_path name;
	_splitpath(src, nullptr, nullptr, name, nullptr);
	return xr_string(name);
}

xr_string	EFS_Utils::ExtractFileExt(const char* src)
{
	string_path ext;
	_splitpath(src, nullptr, nullptr, nullptr, ext);
	return xr_string(ext);
}

xr_string	EFS_Utils::ExtractFilePath(const char* src)
{
	string_path drive, dir;
	_splitpath(src, drive, dir, nullptr, nullptr);
	return xr_string(drive) + dir;
}

xr_string	EFS_Utils::ExcludeBasePath(const char* full_path, const char* excl_path)
{
	const char* sub = strstr(full_path, excl_path);
	if (nullptr != sub) 	return xr_string(sub + xr_strlen(excl_path));
	else	   		return xr_string(full_path);
}

xr_string	EFS_Utils::ChangeFileExt(const char* src, const char* ext)
{
	xr_string	tmp;
	LPSTR src_ext = strext(src);
	if (src_ext) {
		size_t		ext_pos = src_ext - src;
		tmp.assign(src, 0, ext_pos);
	}
	else {
		tmp = src;
	}
	tmp += ext;
	return tmp;
}

xr_string	EFS_Utils::ChangeFileExt(const xr_string& src, const char* ext)
{
	return ChangeFileExt(src.c_str(), ext);
}

//----------------------------------------------------
void MakeFilter(string1024& dest, const char* info, const char* ext)
{
	std::string res;

	if (ext)
	{
		res += info;
		res += "(";
		res += ext;
		res += ")|";
		res += ext;
		res += "|";
		int icnt = _GetItemCount(ext, ';');
		if (icnt > 1)
		{
			for (int idx = 0; idx < icnt; ++idx)
			{
				string64		buf;
				_GetItem(ext, idx, buf, ';');

				res += info;
				res += "(";
				res += buf;
				res += ")|";
				res += buf;
				res += "|";
			}
		}
		res += "|";
	}
	else
	{
		res = "All files(*.*)|*.*||";
	}
	xr_strcpy(dest, res.c_str());

	for (u32 i = 0; i < res.size(); ++i)
	{
		if (res[i] == '|')
			dest[i] = '\0';
	}


}

//------------------------------------------------------------------------------
// start_flt_ext = -1-all 0..n-indices
//------------------------------------------------------------------------------

// Vista uses this hook for old-style save dialog
UINT_PTR CALLBACK OFNHookProcOldStyle(HWND, UINT, WPARAM, LPARAM)
{
	// let default hook work on this message
	return 0;
}


#if !defined(IXR_WINDOWS)
namespace
{
	// SDL renders the native file dialog on every platform; the calls are asynchronous, so the
	// editor keeps pumping events until the user answers and then returns like the Win32 one.
	struct SFileDialogAnswer
	{
		bool Answered = false;
		xr_string Path;
	};

	void FileDialogCallback(void* UserData, const char* const* FileList, int)
	{
		SFileDialogAnswer* Answer = static_cast<SFileDialogAnswer*>(UserData);
		Answer->Answered = true;

		if (FileList != nullptr && FileList[0] != nullptr)
		{
			Answer->Path = FileList[0];
		}
	}

	// "*.dds; *.tga" is turned into the pattern SDL expects: "*.dds;*.tga".
	xr_string MakeDialogPattern(const char* Ext)
	{
		xr_string Pattern = (Ext && Ext[0]) ? Ext : "*";

		for (size_t Index = 0; Index < Pattern.size(); ++Index)
		{
			if (Pattern[Index] == ' ')
			{
				Pattern.erase(Index--, 1);
			}
		}

		return Pattern;
	}

	void WaitForFileDialog(SFileDialogAnswer& Answer)
	{
		while (!Answer.Answered)
		{
			SDL_PumpEvents();
			SDL_Delay(10);
		}
	}
}
#endif

bool EFS_Utils::GetOpenNameInternal(const char* initial, LPSTR buffer, int sz_buf, bool bMulti, const char* offset, int start_flt_ext, const char* ext)
{
#ifdef IXR_WINDOWS
	VERIFY(buffer && (sz_buf > 0));
	FS_Path& P = *FS.get_path(initial);
	string1024 			flt;
	MakeFilter(flt, P.m_FilterCaption ? P.m_FilterCaption : "", ext ? ext : P.m_DefExt);

	OPENFILENAMEA 		ofn;
	memset(&ofn, 0, sizeof(ofn));

	if (xr_strlen(buffer))
	{
		string_path		dr;
		if (!(buffer[0] == '\\' && buffer[1] == '\\')) { // if !network
			_splitpath(buffer, dr, nullptr, nullptr, nullptr);

			if (0 == dr[0])
			{
				string_path		bb;
				P._update(bb, buffer);
				xr_strcpy(buffer, sz_buf, bb);
			}
		}
	}
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = GetForegroundWindow();
	ofn.lpstrDefExt = P.m_DefExt;
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = sz_buf;
	ofn.lpstrFilter = flt;
	ofn.nFilterIndex = start_flt_ext + 2;
	ofn.lpstrTitle = "Open a File";
	string512 path;
	xr_strcpy(path, (offset && offset[0]) ? offset : P.m_Path);
	ofn.lpstrInitialDir = path;

	ofn.Flags = OFN_PATHMUSTEXIST |
		OFN_FILEMUSTEXIST |
		OFN_HIDEREADONLY |
		OFN_FILEMUSTEXIST |
		OFN_NOCHANGEDIR |
		(bMulti ? OFN_ALLOWMULTISELECT | OFN_EXPLORER : 0);

	ofn.FlagsEx = 0;

	bool bRes = !!GetOpenFileNameA(&ofn);
	if (!bRes)
	{
		u32 err = CommDlgExtendedError();
		switch (err)
		{
		case FNERR_BUFFERTOOSMALL:
			Log("Too many files selected.");
			break;
		}
	}
	if (bRes && bMulti)
	{
		int cnt = _GetItemCount(ofn.lpstrFile, 0x0);
		if (cnt > 1)
		{
			char 		dir[255 * 255];
			char 		buf[255 * 255];
			char 		fns[255 * 255];

			xr_strcpy(dir, buffer);
			xr_strcpy(fns, dir);
			xr_strcat(fns, "\\");
			xr_strcat(fns, _GetItem(ofn.lpstrFile, 1, buf, '\0'));

			for (int i = 2; i < cnt; i++)
			{
				xr_strcat(fns, ",");
				xr_strcat(fns, dir);
				xr_strcat(fns, "\\");
				xr_strcat(fns, _GetItem(ofn.lpstrFile, i, buf, '\0'));
			}
			xr_strcpy(buffer, sz_buf, fns);
		}
	}
	strlwr(buffer);
	return bRes;
#else
	FS_Path& P = *FS.get_path(initial);

	const xr_string Pattern = MakeDialogPattern(ext ? ext : P.m_DefExt);
	const SDL_DialogFileFilter Filter = {"Files", Pattern.c_str()};

	SFileDialogAnswer Answer;
	// The engine keeps its paths in the Windows form ("rawdata\levels\"), which the system
	// dialog does not understand, and a directory that does not exist makes it close at once.
	xr_string DefaultLocationStorage = (offset && offset[0]) ? offset : P.m_Path;

	for (char& Ch : DefaultLocationStorage)
	{
		if (Ch == '\\')
		{
			Ch = '/';
		}
	}

	while (!DefaultLocationStorage.empty() && DefaultLocationStorage.back() == '/')
	{
		DefaultLocationStorage.pop_back();
	}

	const char* DefaultLocation = FS.path_exist(DefaultLocationStorage.c_str()) ? DefaultLocationStorage.c_str() : nullptr;

	SDL_ShowOpenFileDialog(FileDialogCallback, &Answer, g_AppInfo.Window, &Filter, 1, DefaultLocation, bMulti);
	WaitForFileDialog(Answer);

	if (Answer.Path.empty())
	{
		return false;
	}

	xr_strcpy(buffer, sz_buf, Answer.Path.c_str());
	return true;
#endif
}


bool EFS_Utils::GetOpenName(const char* initial, xr_string& buffer, bool bMulti, const char* offset, int start_flt_ext, const char* ext)
{
	char			buf[255 * 255]; //max files to select
	xr_strcpy(buf, buffer.c_str());

	bool bRes = GetOpenNameInternal(initial, buf, sizeof(buf), bMulti, offset, start_flt_ext, ext);

	if (bRes)
		buffer = (char*)buf;

	return bRes;
}

bool EFS_Utils::GetSaveName(const char* initial, string_path& buffer, const char* offset, int start_flt_ext, const char* ext)
{
#ifdef IXR_WINDOWS
	FS_Path& P = *FS.get_path(initial);
	string1024 			flt;

	const char* def_ext = P.m_DefExt;

	MakeFilter(flt, P.m_FilterCaption ? P.m_FilterCaption : "", ext ? ext : def_ext);
	OPENFILENAMEA ofn;
	memset(&ofn, 0, sizeof(ofn));
	if (xr_strlen(buffer)) {
		string_path		dr;
		if (!(buffer[0] == '\\' && buffer[1] == '\\')) { // if !network
			_splitpath(buffer, dr, nullptr, nullptr, nullptr);
			if (0 == dr[0])	P._update(buffer, buffer);
		}
	}
	ofn.hwndOwner = GetForegroundWindow();
	ofn.lpstrDefExt = def_ext;
	ofn.lpstrFile = buffer;
	ofn.lpstrFilter = flt;
	ofn.lStructSize = sizeof(ofn);
	ofn.nMaxFile = sizeof(buffer);
	ofn.nFilterIndex = start_flt_ext + 2;
	ofn.lpstrTitle = "Save a File";
	string512 path; xr_strcpy(path, (offset && offset[0]) ? offset : P.m_Path);
	ofn.lpstrInitialDir = path;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER;
	ofn.FlagsEx = 0;


	bool bRes = !!GetSaveFileNameA(&ofn);
	if (!bRes) {
		u32 err = CommDlgExtendedError();
		switch (err) {
		case FNERR_BUFFERTOOSMALL: 	Log("Too many file selected."); break;
		}
	}
	strlwr(buffer);
	return bRes;
#else
	FS_Path& P = *FS.get_path(initial);

	const xr_string Pattern = MakeDialogPattern(ext ? ext : P.m_DefExt);
	const SDL_DialogFileFilter Filter = {"Files", Pattern.c_str()};

	SFileDialogAnswer Answer;
	string_path DefaultLocation = {};
	const char* Start = (offset && offset[0]) ? offset : P.m_Path;
	xr_strcpy(DefaultLocation, Start);

	// The dialog gets the folder to start in; the name itself is typed into the dialog.
	char* LastSlash = strrchr(DefaultLocation, '/');
	if (LastSlash != nullptr && DefaultLocation[strlen(DefaultLocation) - 1] != '/')
	{
		// A file name was passed in: keep it, the dialog uses it as the suggested name.
	}

	SDL_ShowSaveFileDialog(FileDialogCallback, &Answer, g_AppInfo.Window, &Filter, 1, DefaultLocation);
	WaitForFileDialog(Answer);

	if (Answer.Path.empty())
	{
		return false;
	}

	xr_strcpy(buffer, sizeof(buffer), Answer.Path.c_str());
	return true;
#endif
}


bool EFS_Utils::GetSaveName(const char* initial, xr_string& buffer, const char* offset, int start_flt_ext, const char* ext)
{
	string_path				buf;
	xr_strcpy(buf, sizeof(buf), buffer.c_str());
	bool bRes = GetSaveName(initial, buf, offset, start_flt_ext, ext);
	if (bRes)
		buffer = buf;

	return bRes;
}

xr_string EFS_Utils::AppendFolderToName(xr_string& tex_name, int depth, bool full_name)
{
	string1024 nm;
	xr_strcpy(nm, tex_name.c_str());
	tex_name = AppendFolderToName(nm, sizeof(nm), depth, full_name);
	return tex_name;
}

void EFS_Utils::MarkFile(const char* fn, bool bDeleteSource)
{
	xr_string ext = strext(fn);
	ext.insert(1, "~");
	xr_string backup_fn = EFS.ChangeFileExt(fn, ext.c_str());
	if (bDeleteSource) {
		FS.file_rename(fn, backup_fn.c_str(), true);
	}
	else {
		FS.file_copy(fn, backup_fn.c_str());
	}
}

//----------------------------------------------------
const char* EFS_Utils::AppendFolderToName(LPSTR tex_name, u32 const tex_name_size, int depth, bool full_name)
{
	string256 _fn;
	xr_strcpy(tex_name, tex_name_size, AppendFolderToName(tex_name, _fn, sizeof(_fn), depth, full_name));
	return tex_name;
}

const char* EFS_Utils::AppendFolderToName(const char* src_name, LPSTR dest_name, u32 const dest_name_size, int depth, bool full_name)
{
	shared_str tmp = src_name;
	const char* s = src_name;
	LPSTR d = dest_name;
	int sv_depth = depth;
	for (; *s && depth; s++, d++) {
		if (*s == '_') { depth--; *d = '\\'; }
		else { *d = *s; }
	}
	if (full_name) {
		*d = 0;
		if (depth < sv_depth)	xr_strcat(dest_name, dest_name_size, *tmp);
	}
	else {
		for (; *s; s++, d++) *d = *s;
		*d = 0;
	}
	return dest_name;
}

const char* EFS_Utils::GenerateName(const char* base_path, const char* base_name, const char* def_ext, LPSTR out_name, u32 const out_name_size)
{
	int cnt = 0;
	string_path fn;
	if (base_name)
		xr_strconcat(fn, base_path, base_name, def_ext);
	else
		xr_sprintf(fn, sizeof(fn), "%s%02d%s", base_path, cnt++, def_ext);

	while (FS.exist(fn))
		if (base_name)
			xr_sprintf(fn, sizeof(fn), "%s%s%02d%s", base_path, base_name, cnt++, def_ext);
		else
			xr_sprintf(fn, sizeof(fn), "%s%02d%s", base_path, cnt++, def_ext);
	xr_strcpy(out_name, out_name_size, fn);
	return out_name;
}

//#endif
