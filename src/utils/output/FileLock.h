#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB FileLock final {
public:
	FileLock(const std::wstring& filepath);
	~FileLock();

    bool lock();
    void unlock();

private:
    const std::wstring mFilename;
    int mHandle;
};
} // namespace PR