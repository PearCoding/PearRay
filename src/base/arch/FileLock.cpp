#include "FileLock.h"
#include "Logger.h"
#include "Platform.h"

#ifndef PR_OS_WINDOWS
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

namespace PR {
struct FileLockInternal {
	std::filesystem::path Filename;

#ifndef PR_OS_WINDOWS
	int Handle = -1;
#else
	HANDLE Handle = INVALID_HANDLE_VALUE;
#endif
};

FileLock::FileLock(const std::filesystem::path& filepath)
	: mInternal(new FileLockInternal())
{
	mInternal->Filename = filepath;
}

FileLock::~FileLock()
{
	unlock();
}

bool FileLock::lock()
{
	const auto path = mInternal->Filename.c_str();
#ifndef PR_OS_WINDOWS
	mode_t m		  = umask(0);
	mInternal->Handle = open(path, O_RDWR | O_CREAT, 0666);
	umask(m);
	if (mInternal->Handle >= 0 && flock(mInternal->Handle, LOCK_EX | LOCK_NB) < 0) {
		close(mInternal->Handle);
		mInternal->Handle = -1;
	}
	return mInternal->Handle >= 0;
#else
	mInternal->Handle = CreateFileW(path, GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);
	return (mInternal->Handle != INVALID_HANDLE_VALUE);
#endif
}

void FileLock::unlock()
{
#ifndef PR_OS_WINDOWS
	if (mInternal->Handle < 0)
		return;

	const auto path = mInternal->Filename.c_str();
	remove(path);
	close(mInternal->Handle);
	mInternal->Handle = -1;
#else
	if (mInternal->Handle == INVALID_HANDLE_VALUE)
		return;
	CloseHandle(mInternal->Handle);
	mInternal->Handle = INVALID_HANDLE_VALUE;
#endif
}
} // namespace PR