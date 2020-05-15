#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB_BASE FileLock final {
public:
	FileLock(const std::wstring& filepath);
	~FileLock();

    bool lock();
    void unlock();

private:
	std::unique_ptr<struct FileLockInternal> mInternal;
};
} // namespace PR