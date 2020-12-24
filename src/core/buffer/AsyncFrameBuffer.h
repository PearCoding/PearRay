#pragma once

#include "FrameBuffer.h"
#include <vector>

namespace PR {
template <typename T>
class AsyncFrameBuffer {
	PR_CLASS_NON_COPYABLE(AsyncFrameBuffer);

public:
	inline AsyncFrameBuffer(Size1i channels, const Size2i& size,
							const T& clear_value = T(), bool never_clear = false)
		: mFrameBuffer(channels, size, clear_value, never_clear)
		, mLocks(size)
	{
	}

	inline ~AsyncFrameBuffer()
	{
	}

	inline const Size2i& size() const { return mFrameBuffer.size(); }

	inline Size1i width() const { return mFrameBuffer.width(); }
	inline Size1i widthPitch() const { return mFrameBuffer.widthPitch(); }
	inline Size1i widthBytePitch() const { return mFrameBuffer.widthBytePitch(); }

	inline Size1i height() const { return mFrameBuffer.height(); }
	inline Size1i heightPitch() const { return mFrameBuffer.heightPitch(); }
	inline Size1i heightBytePitch() const { return mFrameBuffer.heightBytePitch(); }

	inline Size1i channels() const { return mFrameBuffer.channels(); }
	inline Size1i channelPitch() const { return mFrameBuffer.channelPitch(); }
	inline Size1i channelBytePitch() const { return mFrameBuffer.channelBytePitch(); }

	inline void setNeverClear(bool b) { mFrameBuffer.setNeverClear(b); }
	inline bool isNeverCleared() const { return mFrameBuffer.isNeverCleared(); }

	inline void setFragment(const Point2i& p, Size1i channel, const T& v)
	{
		mLocks.lock(p);
		mFrameBuffer.setFragment(p, channel, v);
		mLocks.unlock(p);
	}

	inline T getFragment(const Point2i& p, Size1i channel) const
	{
		mLocks.lock(p);
		const T ret = mFrameBuffer.getFragment(p, channel);
		mLocks.unlock(p);
		return ret;
	}

	inline void setFragment(Size1i pixelindex, Size1i channel, const T& v)
	{
		mLocks.lock(p);
		mFrameBuffer.setFragment(pixelindex, channel, v);
		mLocks.unlock(p);
	}

	inline T getFragment(Size1i pixelindex, Size1i channel = 0) const
	{
		mLocks.lock(p);
		const T ret = mFrameBuffer.getFragment(p, channel);
		mLocks.unlock(p);
		return ret;
	}

	inline const T* ptr() const { return mData.data(); }
	inline T* ptr() { return mData.data(); }

	inline void clearUnsafe(bool force = false)
	{
		mFrameBuffer.clear(force);
	}

	inline void fillUnsafe(const T& v)
	{
		mFrameBuffer.fill(v);
	}

private:
	FrameBuffer<T> mFrameBuffer;
	PixelLock mLocks;
};

typedef AsyncFrameBuffer<float> AsyncFrameBufferFloat;
typedef AsyncFrameBuffer<uint32> AsyncFrameBufferUInt32;
} // namespace PR
