#include "FileLock.h"

// TODO: Add windows support!
#ifndef PR_OS_WINDOWS
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <boost/filesystem.hpp>

namespace PR {
FileLock::FileLock(const std::wstring& filepath)
	: mFilename(filepath)
	, mHandle(-1)
{
}

FileLock::~FileLock()
{
	unlock();
}

bool FileLock::lock()
{
#ifndef PR_OS_WINDOWS
	const auto path = boost::filesystem::path(mFilename);

	mode_t m = umask(0);
	mHandle	 = open(path.c_str(), O_RDWR | O_CREAT, 0666);
	umask(m);
	if (mHandle >= 0 && flock(mHandle, LOCK_EX | LOCK_NB) < 0) {
		close(mHandle);
		mHandle = -1;
	}
#endif

	return mHandle >= 0;
}

void FileLock::unlock()
{
	if (mHandle < 0)
		return;

#ifndef PR_OS_WINDOWS
	const auto path = boost::filesystem::path(mFilename);
	remove(path.c_str());
	close(mHandle);
	mHandle = -1;
#endif
}
} // namespace PR