#include "xr_file_system_posix.h"
#include "xr_string_utils.h"

#include <sys/stat.h>
#include <strings.h>

using namespace xray_re;

// The Windows build of this class lives in xr_file_system_win32.cxx; the functions below are
// the ones that talk to the operating system, so they have a plain POSIX implementation.

void xr_file_system::split_path(const char* path, std::string* folder,
		std::string* name, std::string* extension)
{
	xr_assert(path);

	const char* last_slash = strrchr(path, '/');
	const char* base = last_slash ? last_slash + 1 : path;
	const char* last_dot = strrchr(base, '.');

	if (folder) {
		folder->assign(path, last_slash ? size_t(last_slash - path + 1) : 0);
		xr_strlwr(*folder);
	}
	if (name) {
		name->assign(base, last_dot ? size_t(last_dot - base) : strlen(base));
		xr_strlwr(*name);
	}
	if (extension) {
		extension->assign(last_dot ? last_dot : "");
		xr_strlwr(*extension);
	}
}

bool xr_file_system::folder_exist(const char* path)
{
	struct stat st;
	if (::stat(path, &st) != 0)
		return false;
	return S_ISDIR(st.st_mode) != 0;
}

bool xr_file_system::file_exist(const char* path)
{
	struct stat st;
	if (::stat(path, &st) != 0)
		return false;
	return S_ISREG(st.st_mode) != 0;
}

size_t xr_file_system::file_length(const char* path)
{
	struct stat st;
	if (::stat(path, &st) != 0)
		return 0;
	return size_t(st.st_size);
}

uint32_t xr_file_system::file_age(const char* path)
{
	struct stat st;
	if (::stat(path, &st) != 0)
		return 0;
	return uint32_t(st.st_mtime);
}

bool xr_file_system::create_folder(const char* path) const
{
	if (::mkdir(path, 0755) == 0)
		return true;
	return errno == EEXIST && folder_exist(path);
}

namespace
{
	class xr_file_writer_posix: public xr_writer
	{
	public:
		explicit xr_file_writer_posix(FILE* file): m_file(file) {}
		virtual ~xr_file_writer_posix() { if (m_file != nullptr) fclose(m_file); }

		virtual void w_raw(const void* src, size_t src_size) override
		{
			fwrite(src, 1, src_size, m_file);
		}

		virtual void seek(size_t pos) override
		{
			fseek(m_file, long(pos), SEEK_SET);
		}

		virtual size_t tell() override
		{
			return size_t(ftell(m_file));
		}

	private:
		FILE* m_file;
	};
}

xr_reader* xr_file_system::r_open(const char* path) const
{
	FILE* file = fopen(path, "rb");
	if (file == nullptr)
		return nullptr;

	fseek(file, 0, SEEK_END);
	const long length = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (length < 0) {
		fclose(file);
		return nullptr;
	}

	uint8_t* data = static_cast<uint8_t*>(malloc(size_t(length)));
	if (data == nullptr) {
		fclose(file);
		return nullptr;
	}

	const size_t read = fread(data, 1, size_t(length), file);
	fclose(file);
	if (read != size_t(length)) {
		free(data);
		return nullptr;
	}

	// xr_temp_reader takes ownership of the buffer.
	return new xr_temp_reader(data, size_t(length));
}

xr_writer* xr_file_system::w_open(const char* path, bool ignore_ro) const
{
	if (!ignore_ro && read_only()) {
		dbg("fs_ro: writing %s", path);
		return new xr_fake_writer();
	}

	FILE* file = fopen(path, "wb");
	if (file == nullptr) {
		std::string folder;
		split_path(path, &folder);
		if (!create_path(folder))
			return nullptr;
		file = fopen(path, "wb");
		if (file == nullptr)
			return nullptr;
	}
	return new xr_file_writer_posix(file);
}

bool xr_file_system::create_path(const char* path) const
{
	const std::string full(path);
	std::string part;
	size_t pos = 0;

	// Walk the components and create the missing ones, like the Windows build does.
	while (pos < full.size()) {
		const size_t slash = full.find('/', pos + 1);
		part.assign(full, 0, slash == std::string::npos ? std::string::npos : slash);

		if (!part.empty() && !folder_exist(part.c_str())) {
			if (!create_folder(part.c_str()))
				return false;
		}

		if (slash == std::string::npos)
			break;
		pos = slash;
	}
	return true;
}

bool xr_file_system::copy_file(const char* src_path, const char* tgt_path) const
{
	FILE* src = fopen(src_path, "rb");
	if (src == nullptr)
		return false;

	FILE* tgt = fopen(tgt_path, "wb");
	if (tgt == nullptr) {
		fclose(src);
		return false;
	}

	char buffer[64 * 1024];
	size_t read;
	bool ok = true;
	while ((read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
		if (fwrite(buffer, 1, read, tgt) != read) {
			ok = false;
			break;
		}
	}

	fclose(src);
	fclose(tgt);
	return ok;
}
