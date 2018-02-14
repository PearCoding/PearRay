#pragma once

#include "renderer/RenderContext.h"

#include <Eigen/Dense>

namespace PR {
class RenderContext;

template <typename T>
class PR_LIB FrameBuffer {
	PR_CLASS_NON_COPYABLE(FrameBuffer);

public:
	inline explicit FrameBuffer(size_t channels, const T& clear_value = T(), bool never_clear = false)
		: mWidth(0)
		, mHeight(0)
		, mOffsetX(0)
		, mOffsetY(0)
		, mChannels(channels)
		, mData(nullptr)
		, mClearValue(clear_value)
		, mNeverClear(never_clear)
	{
	}

	inline ~FrameBuffer()
	{
		deinit();
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

	inline void setNeverClear(bool b)
	{
		mNeverClear = b;
	}

	inline bool isNeverCleared() const
	{
		return mNeverClear;
	}

	inline void init(RenderContext* renderer)
	{
		PR_ASSERT(renderer, "Renderer should be not null");
		PR_ASSERT(!mData, "Init shouldn't be called twice");

		mWidth   = renderer->width();
		mHeight  = renderer->height();
		mOffsetX = renderer->offsetX();
		mOffsetY = renderer->offsetY();

		PR_ASSERT(mWidth > mOffsetX, "Offset should be less then width");
		PR_ASSERT(mHeight > mOffsetY, "Offset should be less then height");

		mData = new T[mWidth * mHeight * mChannels];
		clear(true);
	}

	inline void deinit()
	{
		if (mData) {
			delete[] mData;
			mData = nullptr;
		}
	}

	inline void setFragment(const Eigen::Vector2i& p, size_t channel, const T& v) const
	{
		PR_ASSERT(p(0) >= 0 && (uint32) p(0) >= mOffsetX && (uint32) p(0) < mOffsetX + mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) >= 0 && (uint32) p(1) >= mOffsetY && (uint32) p(1) < mOffsetY + mHeight,
				  "y coord has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		mData[(p(1) - mOffsetY) * mWidth * mChannels + (p(0) - mOffsetX) * mChannels + channel] = v;
	}

	inline const T& getFragment(const Eigen::Vector2i& p, size_t channel=0) const
	{
		PR_ASSERT(p(0) >= 0 && (uint32) p(0) >= mOffsetX && (uint32) p(0) < mOffsetX + mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) >= 0 && (uint32) p(1) >= mOffsetY && (uint32) p(1) < mOffsetY + mHeight,
				  "y coord has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		return mData[(p(1) - mOffsetY) * mWidth * mChannels + (p(0) - mOffsetX) * mChannels + channel];
	}

	inline T& getFragment(const Eigen::Vector2i& p, size_t channel=0)
	{
		PR_ASSERT(p(0) >= 0 && (uint32) p(0) >= mOffsetX && (uint32) p(0) < mOffsetX + mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) >= 0 && (uint32) p(1) >= mOffsetY && (uint32) p(1) < mOffsetY + mHeight,
				  "y coord has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		return mData[(p(1) - mOffsetY) * mWidth * mChannels + (p(0) - mOffsetX) * mChannels + channel];
	}

	inline void setFragmentBounded(const Eigen::Vector2i& p, size_t channel, const T& v)
	{
		PR_ASSERT(p(0) >= 0 && (uint32) p(0) < mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) >= 0 && (uint32) p(1) < mHeight,
				  "y coord has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		mData[p(1) * mWidth * mChannels + p(0) * mChannels + channel] = v;
	}

	inline const T& getFragmentBounded(const Eigen::Vector2i& p, size_t channel=0) const
	{
		PR_ASSERT(p(0) >= 0 && (uint32) p(0) < mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) >= 0 && (uint32) p(1) < mHeight,
				  "y coord has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		return mData[p(1) * mWidth * mChannels + p(0) * mChannels + channel];
	}

	inline T& getFragmentBounded(const Eigen::Vector2i& p, size_t channel=0)
	{
		PR_ASSERT(p(0) >= 0 && (uint32) p(0) < mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) >= 0 && (uint32) p(1) < mHeight,
				  "y coord has to be between boundaries");
		PR_ASSERT(channel < mChannels,
				  "channel coord has to be less then maximum");
		return mData[p(1) * mWidth * mChannels + p(0) * mChannels + channel];
	}

	// Spectrum
	inline void getFragment(const Eigen::Vector2i& p, Spectrum& spec) {
		PR_ASSERT(spec.samples() == mChannels, "Channel count and spectrum samples have to be the same");
		for(uint32 i = 0; i < mChannels; ++i)
			spec[i] = getFragment(p, i);
	}

	inline void getFragmentBounded(const Eigen::Vector2i& p, Spectrum& spec) {
		PR_ASSERT(spec.samples() == mChannels, "Channel count and spectrum samples have to be the same");
		for(uint32 i = 0; i < mChannels; ++i)
			spec[i] = getFragmentBounded(p, i);
	}

	inline void setFragment(const Eigen::Vector2i& p, const Spectrum& spec) {
		PR_ASSERT(spec.samples() == mChannels, "Channel count and spectrum samples have to be the same");
		spec.copyTo(&getFragment(p));
	}

	inline void setFragmentBounded(const Eigen::Vector2i& p, const Spectrum& spec) {
		PR_ASSERT(spec.samples() == mChannels, "Channel count and spectrum samples have to be the same");
		spec.copyTo(&getFragmentBounded(p));
	}

	inline T* ptr() const
	{
		return mData;
	}

	inline void clear(bool force = false)
	{
		if (force || !mNeverClear)
			fill(mClearValue);
	}

	inline void fill(const T& v)
	{
		std::fill_n(mData, mWidth * mHeight * mChannels, v);
	}

private:
	size_t mWidth;
	size_t mHeight;
	size_t mOffsetX;
	size_t mOffsetY;
	size_t mChannels;
	T* mData;
	T mClearValue;
	bool mNeverClear;
};

typedef FrameBuffer<float> FrameBufferFloat;
typedef FrameBuffer<uint64> FrameBufferUInt64;
}
