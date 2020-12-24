#pragma once

#include "PR_Config.h"
#include <mutex>

namespace PR {
class PR_LIB_CORE PixelLock {
public:
	inline explicit PixelLock(const Size2i& size)
		: mSize(size)
		, mLocks(size.area())
	{
	}
	inline ~PixelLock() = default;

	inline bool tryLock(const Point2i& p)
	{
		PR_ASSERT(p(0) < mSize.Width,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) < mSize.Height,
				  "y coord has to be between boundaries");

		return tryLock(p(1) * mSize.Width + p(0));
	}

	inline void lock(const Point2i& p)
	{
		PR_ASSERT(p(0) < mSize.Width,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) < mSize.Height,
				  "y coord has to be between boundaries");
		lock(p(1) * mSize.Width + p(0));
	}

	inline void unlock(const Point2i& p)
	{
		PR_ASSERT(p(0) < mSize.Width,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) < mSize.Height,
				  "y coord has to be between boundaries");
		unlock(p(1) * mSize.Width + p(0));
	}

	inline bool tryLock(Size1i pixelindex)
	{
		PR_ASSERT(pixelindex < mSize.area(),
				  "pixelindex has to be between boundaries");
		return mLocks[pixelindex].try_lock();
	}

	inline void lock(Size1i pixelindex)
	{
		PR_ASSERT(pixelindex < mSize.area(),
				  "pixelindex has to be between boundaries");
		mLocks[pixelindex].lock();
	}

	inline void unlock(Size1i pixelindex)
	{
		PR_ASSERT(pixelindex < mSize.area(),
				  "pixelindex has to be between boundaries");
		mLocks[pixelindex].unlock();
	}

private:
	const Size2i mSize;
	std::vector<std::mutex> mLocks;
};
} // namespace PR
