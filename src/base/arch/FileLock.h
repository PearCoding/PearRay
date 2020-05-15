#pragma once

#include "PR_Config.h"

namespace PR {
/// Locked file to ensure only one raytracers works on one scene
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