#pragma once

#include "PR_Config.h"
#include <vector>

namespace PR {
template <typename T>
class PR_LIB FrameBuffer {
	PR_CLASS_NON_COPYABLE(FrameBuffer);

public:
	inline FrameBuffer(size_t channels, size_t width, size_t height,
					   const T& clear_value = T(), bool never_clear = false)
		: mWidth(width)
		, mHeight(height)
		, mChannels(channels)
		, mClearValue(clear_value)
		, mNeverClear(never_clear)
		, mData()
	{
		mData.resize(mWidth * mHeight * mChannels);
	}

	inline ~FrameBuffer()
	{
	}

	inline size_t width() const { return mWidth; }
	inline size_t widthPitch() const { return channels() * channelPitch(); }
	inline size_t widthBytePitch() const { return widthPitch() * sizeof(T); }

	inline size_t height() const { return mHeight; }
	inline size_t heightPitch() const { return width() * widthPitch(); }
	inline size_t heightBytePitch() const { return heightPitch() * sizeof(T); }

	inline size_t channels() const { return mChannels; }
	inline size_t channelPitch() const { return 1; }
	inline size_t channelBytePitch() const { return channelPitch() * sizeof(T); }

	inline void setNeverClear(bool b) { mNeverClear = b; }
	inline bool isNeverCleared() const { return mNeverClear; }

	inline void setFragment(size_t px, size_t py, size_t channel, const T& v)
	{
		PR_ASSERT(px < mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(py < mHeight,
				  "y coord has to be between boundaries");
		setFragment(py * mWidth + px, channel, v);
	}

	inline const T& getFragment(size_t px, size_t py, size_t channel) const
	{
		PR_ASSERT(px < mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(py < mHeight,
				  "y coord has to be between boundaries");
		return getFragment(py * mWidth + px, channel);
	}

	inline T& getFragment(size_t px, size_t py, size_t channel)
	{
		PR_ASSERT(px < mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(py < mHeight,
				  "y coord has to be between boundaries");
		return getFragment(py * mWidth + px, channel);
	}

	inline void setFragment(size_t pixelindex, size_t channel, const T& v)
	{
		PR_ASSERT(pixelindex < mWidth * mHeight,
				  "pixelindex has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		getFragment(pixelindex, channel) = v;
	}

	inline const T& getFragment(size_t pixelindex, size_t channel = 0) const
	{
		PR_ASSERT(pixelindex < mWidth * mHeight,
				  "pixelindex has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		return mData[channel + pixelindex * widthPitch()];
	}

	inline T& getFragment(size_t pixelindex, size_t channel = 0)
	{
		PR_ASSERT(pixelindex < mWidth * mHeight,
				  "pixelindex has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		return mData[channel + pixelindex * widthPitch()];
	}

	inline void blendFragment(uint32 pixelIndex, size_t channel,
							  const T& newValue, float t)
	{
		getFragment(pixelIndex, channel) = getFragment(pixelIndex, channel) * (1 - t)
										   + newValue * t;
	}

	inline const T* ptr() const { return mData.data(); }
	inline T* ptr() { return mData.data(); }

	inline void clear(bool force = false)
	{
		if (force || !mNeverClear)
			fill(mClearValue);
	}

	inline void fill(const T& v)
	{
		std::fill(mData.begin(), mData.end(), v);
	}

private:
	size_t mWidth;
	size_t mHeight;
	size_t mChannels;
	T mClearValue;
	bool mNeverClear;

	std::vector<T> mData;
};

typedef FrameBuffer<float> FrameBufferFloat;
typedef FrameBuffer<uint32> FrameBufferUInt32;
} // namespace PR
