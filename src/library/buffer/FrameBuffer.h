#pragma once

#include "PR_Config.h"
#include <vector>

namespace PR {
template <typename T>
class FrameBuffer {
	PR_CLASS_NON_COPYABLE(FrameBuffer);

public:
	inline FrameBuffer(Size1i channels, const Size2i& size,
					   const T& clear_value = T(), bool never_clear = false)
		: mSize(size)
		, mChannels(channels)
		, mClearValue(clear_value)
		, mNeverClear(never_clear)
		, mData()
	{
		mData.resize(mSize.area() * mChannels);
	}

	inline ~FrameBuffer()
	{
	}

	inline const Size2i& size() const { return mSize; }

	inline Size1i width() const { return mSize.Width; }
	inline Size1i widthPitch() const { return channels() * channelPitch(); }
	inline Size1i widthBytePitch() const { return widthPitch() * sizeof(T); }

	inline Size1i height() const { return mSize.Height; }
	inline Size1i heightPitch() const { return width() * widthPitch(); }
	inline Size1i heightBytePitch() const { return heightPitch() * sizeof(T); }

	inline Size1i channels() const { return mChannels; }
	inline Size1i channelPitch() const { return 1; }
	inline Size1i channelBytePitch() const { return channelPitch() * sizeof(T); }

	inline void setNeverClear(bool b) { mNeverClear = b; }
	inline bool isNeverCleared() const { return mNeverClear; }

	inline void setFragment(const Point2i& p, Size1i channel, const T& v)
	{
		PR_ASSERT(p(0) < mSize.Width,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) < mSize.Height,
				  "y coord has to be between boundaries");
		setFragment(p(1) * mSize.Width + p(0), channel, v);
	}

	inline const T& getFragment(const Point2i& p, Size1i channel) const
	{
		PR_ASSERT(p(0) < mSize.Width,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) < mSize.Height,
				  "y coord has to be between boundaries");
		return getFragment(p(1) * mSize.Width + p(0), channel);
	}

	inline T& getFragment(const Point2i& p, Size1i channel)
	{
		PR_ASSERT(p(0) < mSize.Width,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) < mSize.Height,
				  "y coord has to be between boundaries");
		return getFragment(p(1) * mSize.Width + p(0), channel);
	}

	inline void setFragment(Size1i pixelindex, Size1i channel, const T& v)
	{
		PR_ASSERT(pixelindex < mSize.area(),
				  "pixelindex has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		getFragment(pixelindex, channel) = v;
	}

	inline const T& getFragment(Size1i pixelindex, Size1i channel = 0) const
	{
		PR_ASSERT(pixelindex < mSize.area(),
				  "pixelindex has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		return mData[channel + pixelindex * widthPitch()];
	}

	inline T& getFragment(Size1i pixelindex, Size1i channel = 0)
	{
		PR_ASSERT(pixelindex < mSize.Width * mSize.Height,
				  "pixelindex has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		return mData[channel + pixelindex * widthPitch()];
	}

	inline void blendFragment(const Point2i& p, Size1i channel,
							  const T& newValue, float t)
	{
		T& val = getFragment(p, channel);
		val	= val * (1 - t) + newValue * t;
	}

	inline void blendFragment(Size1i pixelIndex, Size1i channel,
							  const T& newValue, float t)
	{
		T& val = getFragment(pixelIndex, channel);
		val	= val * (1 - t) + newValue * t;
	}

	inline void addBlock(const Point2i& dst_off,
						 const Size2i& dst_size,
						 const Point2i& src_off,
						 const Size2i& src_size,
						 const FrameBuffer<T>& block)
	{
		PR_ASSERT(mChannels == block.mChannels, "Expected equal count of channels");

		Point2i src_end = (src_size + src_off).cwiseMin(block.size().asArray());
		Point2i dst_end = (dst_size + dst_off).cwiseMin(size().asArray());

		Point2i size = (dst_end - dst_off).cwiseMin(src_end - src_off);

		for (Size1i y = 0; y < size.y(); ++y)
			for (Size1i x = 0; x < size.x(); ++x)
				for (Size1i ch = 0; ch < mChannels; ++ch)
					getFragment(dst_off + Point2i(x, y), ch) += block.getFragment(src_off + Point2i(x, y), ch);
	}

	inline void addBlock(const Point2i& dst_off,
						 const Point2i& src_off,
						 const FrameBuffer<T>& block)
	{
		addBlock(dst_off, size(),
				 src_off, block.size(),
				 block);
	}

	inline void addBlock(const Point2i& dst_off,
						 const FrameBuffer<T>& block)
	{
		addBlock(dst_off, Point2i::Zero(), block);
	}

	template <typename Func>
	inline void applyBlock(const Point2i& dst_off,
						   const Size2i& dst_size,
						   const Point2i& src_off,
						   const Size2i& src_size,
						   const FrameBuffer<T>& block,
						   Func func)
	{
		PR_ASSERT(mChannels == block.mChannels, "Expected equal count of channels");

		Point2i src_end = (src_size + src_off).cwiseMin(block.size().asArray());
		Point2i dst_end = (dst_size + dst_off).cwiseMin(size().asArray());

		Point2i size = (dst_end - dst_off).cwiseMin(src_end - src_off);

		for (Size1i y = 0; y < size.y(); ++y)
			for (Size1i x = 0; x < size.x(); ++x)
				for (Size1i ch = 0; ch < mChannels; ++ch)
					getFragment(dst_off + Point2i(x, y), ch) = func(getFragment(dst_off + Point2i(x, y), ch),
																	 block.getFragment(src_off + Point2i(x, y), ch));
	}

	template <typename Func>
	inline void applyBlock(const Point2i& dst_off,
						   const Point2i& src_off,
						   const FrameBuffer<T>& block,
						   Func func)
	{
		applyBlock(dst_off, size(),
				   src_off, block.size(),
				   block,
				   func);
	}

	template <typename Func>
	inline void applyBlock(const Point2i& dst_off,
						   const FrameBuffer<T>& block,
						   Func func)
	{
		applyBlock(dst_off, Point2i::Zero(), block, func);
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
	Size2i mSize;
	Size1i mChannels;
	T mClearValue;
	bool mNeverClear;

	std::vector<T> mData;
};

typedef FrameBuffer<float> FrameBufferFloat;
typedef FrameBuffer<uint32> FrameBufferUInt32;
} // namespace PR
