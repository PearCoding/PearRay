#include "FileLock.h"
#include "Platform.h"
#include "Logger.h"

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
	std::wstring Filename;

#ifndef PR_OS_WINDOWS
	int Handle;
#else
	HANDLE Handle;
#endif
};

FileLock::FileLock(const std::wstring& filepath)
	: mInternal(new FileLockInternal())
{
	mInternal->Filename = filepath;
#ifndef PR_OS_WINDOWS
	mInternal->Handle = -1;
#else
	mInternal->Handle = INVALID_HANDLE_VALUE;
#endif
}

FileLock::~FileLock()
{
	unlock();
}

bool FileLock::lock()
{
	const auto path = encodePath(mInternal->Filename);
#ifndef PR_OS_WINDOWS
	mode_t m		  = umask(0);
	mInternal->Handle = open(path.c_str(), O_RDWR | O_CREAT, 0666);
	umask(m);
	if (mInternal->Handle >= 0 && flock(mInternal->Handle, LOCK_EX | LOCK_NB) < 0) {
		close(mInternal->Handle);
		mInternal->Handle = -1;
	}
	return mInternal->Handle >= 0;
#else
	mInternal->Handle = CreateFileW(path.c_str(), GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);
	return (mInternal->Handle != INVALID_HANDLE_VALUE);
#endif
}

void FileLock::unlock()
{
	const auto path = encodePath(mInternal->Filename);
#ifndef PR_OS_WINDOWS
	if (mInternal->Handle < 0)
		return;

	remove(path.c_str());
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