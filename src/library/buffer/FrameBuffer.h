#pragma once

#include "PR_Config.h"
#include <vector>

namespace PR {
template <typename T>
class FrameBuffer {
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

	inline void blendFragment(size_t px, size_t py, size_t channel,
							  const T& newValue, float t)
	{
		T& val = getFragment(px, py, channel);
		val	= val * (1 - t) + newValue * t;
	}

	inline void blendFragment(uint32 pixelIndex, size_t channel,
							  const T& newValue, float t)
	{
		getFragment(pixelIndex, channel) = getFragment(pixelIndex, channel) * (1 - t)
										   + newValue * t;
	}

	inline void addBlock(size_t dst_x, size_t dst_y,
						 size_t dst_w, size_t dst_h,
						 size_t src_x, size_t src_y,
						 size_t src_w, size_t src_h,
						 const FrameBuffer<T>& block)
	{
		PR_ASSERT(mChannels == block.mChannels, "Expected equal count of channels");

		size_t src_ex = std::min(src_w + src_x, block.width());
		size_t src_ey = std::min(src_h + src_y, block.height());

		size_t dst_ex = std::min(dst_w + dst_x, width());
		size_t dst_ey = std::min(dst_h + dst_y, height());

		size_t w = std::min(dst_ex - dst_x, src_ex - src_x);
		size_t h = std::min(dst_ey - dst_y, src_ey - src_y);

		for (size_t y = 0; y < h; ++y)
			for (size_t x = 0; x < w; ++x)
				for (size_t ch = 0; ch < mChannels; ++ch)
					getFragment(dst_x + x, dst_y + y, ch) += block.getFragment(src_x + x, src_y + y, ch);
	}

	inline void addBlock(size_t dst_x, size_t dst_y,
						 size_t src_x, size_t src_y,
						 const FrameBuffer<T>& block)
	{
		addBlock(dst_x, dst_y,
				 width(), height(),
				 src_x, src_y,
				 block.width(), block.height(),
				 block);
	}

	inline void addBlock(size_t dst_x, size_t dst_y,
						 const FrameBuffer<T>& block)
	{
		addBlock(dst_x, dst_y, 0, 0, block);
	}

	template <typename Func>
	inline void applyBlock(size_t dst_x, size_t dst_y,
						   size_t dst_w, size_t dst_h,
						   size_t src_x, size_t src_y,
						   size_t src_w, size_t src_h,
						   const FrameBuffer<T>& block,
						   Func func)
	{
		PR_ASSERT(mChannels == block.mChannels, "Expected equal count of channels");

		size_t src_ex = std::min(src_w + src_x, block.width());
		size_t src_ey = std::min(src_h + src_y, block.height());

		size_t dst_ex = std::min(dst_w + dst_x, width());
		size_t dst_ey = std::min(dst_h + dst_y, height());

		size_t w = std::min(dst_ex - dst_x, src_ex - src_x);
		size_t h = std::min(dst_ey - dst_y, src_ey - src_y);

		for (size_t y = 0; y < h; ++y)
			for (size_t x = 0; x < w; ++x)
				for (size_t ch = 0; ch < mChannels; ++ch)
					getFragment(dst_x + x, dst_y + y, ch) = func(getFragment(dst_x + x, dst_y + y, ch), block.getFragment(src_x + x, src_y + y, ch));
	}

	template <typename Func>
	inline void applyBlock(size_t dst_x, size_t dst_y,
						   size_t src_x, size_t src_y,
						   const FrameBuffer<T>& block,
						   Func func)
	{
		applyBlock(dst_x, dst_y,
				   width(), height(),
				   src_x, src_y,
				   block.width(), block.height(),
				   block,
				   func);
	}

	template <typename Func>
	inline void applyBlock(size_t dst_x, size_t dst_y,
						   const FrameBuffer<T>& block,
						   Func func)
	{
		applyBlock(dst_x, dst_y, 0, 0, block, func);
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
