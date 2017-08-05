#pragma once

#include "renderer/RenderContext.h"

#include <Eigen/Dense>

namespace PR {
class RenderContext;

template <typename T>
class PR_LIB OutputChannel {
	PR_CLASS_NON_COPYABLE(OutputChannel);

public:
	inline explicit OutputChannel(RenderContext* renderer, const T& clear_value = T(), bool never_clear = false)
		: mRenderer(renderer)
		, mWidth(0)
		, mHeight(0)
		, mData(nullptr)
		, mClearValue(clear_value)
		, mNeverClear(never_clear)
	{
	}

	inline ~OutputChannel()
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

	inline void setNeverClear(bool b)
	{
		mNeverClear = b;
	}

	inline bool isNeverCleared() const
	{
		return mNeverClear;
	}

	inline void init()
	{
		PR_ASSERT(!mData, "Init shouldn't be called twice");
		mWidth = mRenderer->width();
		mHeight = mRenderer->height();
		mData = new T[mWidth*mHeight];
		clear(true);
	}

	inline void deinit()
	{
		if (mData) {
			delete[] mData;
			mData = nullptr;
		}
	}

	inline void setFragment(const Eigen::Vector2i& p, const T& v) const
	{
		PR_ASSERT(p(0) >= mRenderer->offsetX() && p(0) < mRenderer->offsetX() + mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) >= mRenderer->offsetY() && p(1) < mRenderer->offsetY() + mHeight,
				  "y coord has to be between boundaries");
		mData[(p(1) - mRenderer->offsetY()) * mWidth + (p(0) - mRenderer->offsetX())] = v;
	}

	inline const T& getFragment(const Eigen::Vector2i& p) const
	{
		PR_ASSERT(p(0) >= mRenderer->offsetX() && p(0) < mRenderer->offsetX() + mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) >= mRenderer->offsetY() && p(1) < mRenderer->offsetY() + mHeight,
				  "y coord has to be between boundaries");
		return mData[(p(1) - mRenderer->offsetY()) * mWidth + (p(0) - mRenderer->offsetX())];
	}

	inline void setFragmentBounded(const Eigen::Vector2i& p, const T& v)
	{
		mData[p(1) * mWidth + p(0)] = v;
	}

	inline const T& getFragmentBounded(const Eigen::Vector2i& p) const
	{
		return mData[p(1) * mWidth + p(0)];
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
		std::fill_n(mData, mWidth*mHeight, v);
	}

private:
	RenderContext* mRenderer;
	size_t mWidth;
	size_t mHeight;
	T* mData;
	T mClearValue;
	bool mNeverClear;
};

typedef OutputChannel<Eigen::Vector3f> Output3D;
typedef OutputChannel<float> Output1D;
typedef OutputChannel<uint64> OutputCounter;
typedef OutputChannel<Spectrum> OutputSpectral;
}
