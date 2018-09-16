#pragma once

#include "math/SIMD.h"

namespace PR {
/* This framebuffer uses a channel * height * width structure
 * to better utilize SIMD channel independent loading and storing
 * It also expands width and height to be a multiply of the simd bandwidth
 */
template <typename T, typename VT>
class PR_LIB FrameBuffer {
	PR_CLASS_NON_COPYABLE(FrameBuffer);

public:
	inline FrameBuffer(size_t channels, size_t width, size_t height,
					   const T& clear_value = T(), bool never_clear = false)
		: mWidth(width)
		, mHeight(height)
		, mChannels(channels)
		, mChannelPitch(width * height + (width * height) % PR_SIMD_BANDWIDTH)
		, mClearValue(clear_value)
		, mNeverClear(never_clear)
		, mData()
	{
		mData.resize(mChannels * mChannelPitch);
	}

	inline ~FrameBuffer()
	{
	}

	inline size_t width() const
	{
		return mWidth;
	}

	inline size_t height() const
	{
		return mHeight;
	}

	inline size_t channels() const
	{
		return mChannels;
	}

	inline size_t channelPitch() const
	{
		return mChannelPitch;
	}

	inline void setNeverClear(bool b)
	{
		mNeverClear = b;
	}

	inline bool isNeverCleared() const
	{
		return mNeverClear;
	}

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
		mData[channel * mChannelPitch + pixelindex] = v;
	}

	inline const T& getFragment(size_t pixelindex, size_t channel = 0) const
	{
		PR_ASSERT(pixelindex < mWidth * mHeight,
				  "pixelindex has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		return mData[channel * mChannelPitch + pixelindex];
	}

	inline T& getFragment(size_t pixelindex, size_t channel = 0)
	{
		PR_ASSERT(pixelindex < mWidth * mHeight,
				  "pixelindex has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		return mData[channel * mChannelPitch + pixelindex];
	}

	// SIMD versions
	inline void setFragment(const vuint32& pixelIndex, size_t channel, const VT& v)
	{
		store_into_container(pixelIndex, mData, v, channel * mChannelPitch);
	}

	inline VT getFragment(const vuint32& pixelIndex, size_t channel = 0) const
	{
		return load_from_container(pixelIndex, mData, channel * mChannelPitch);
	}

	inline void blendFragment(const vuint32& pixelIndex, size_t channel,
							  const VT& newValue, const vfloat& blendFactor)
	{
		// Better performance then using load and store
		for_each_v([&](uint32 i) {
			uint32 pi = extract(i, pixelIndex);
			T v		  = extract(i, newValue);
			float t   = extract(i, blendFactor);

			getFragment(pi, channel) = getFragment(pi, channel) * (1 - t) + v * t;
		});
	}

	inline const T* ptr() const
	{
		return mData.data();
	}

	inline T* ptr()
	{
		return mData.data();
	}

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
	size_t mChannelPitch;
	T mClearValue;
	bool mNeverClear;

	simd_vector<T> mData;
};

typedef FrameBuffer<float, vfloat> FrameBufferFloat;
typedef FrameBuffer<uint32, vuint32> FrameBufferUInt32;
} // namespace PR
