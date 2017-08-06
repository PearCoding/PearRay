#pragma once

#include "renderer/RenderContext.h"

#include <Eigen/Dense>

namespace PR {
class RenderContext;

template <typename T>
class PR_LIB OutputChannel {
	PR_CLASS_NON_COPYABLE(OutputChannel);

public:
	inline explicit OutputChannel(const T& clear_value = T(), bool never_clear = false)
		: mWidth(0)
		, mHeight(0)
		, mOffsetX(0)
		, mOffsetY(0)
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

		mData = new T[mWidth * mHeight];
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
		PR_ASSERT(p(0) >= mOffsetX && p(0) < mOffsetX + mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) >= mOffsetY && p(1) < mOffsetY + mHeight,
				  "y coord has to be between boundaries");
		mData[(p(1) - mOffsetY) * mWidth + (p(0) - mOffsetX)] = v;
	}

	inline const T& getFragment(const Eigen::Vector2i& p) const
	{
		PR_ASSERT(p(0) >= mOffsetX && p(0) < mOffsetX + mWidth,
				  "x coord has to be between boundaries");
		PR_ASSERT(p(1) >= mOffsetY && p(1) < mOffsetY + mHeight,
				  "y coord has to be between boundaries");
		return mData[(p(1) - mOffsetY) * mWidth + (p(0) - mOffsetX)];
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
		std::fill_n(mData, mWidth * mHeight, v);
	}

private:
	size_t mWidth;
	size_t mHeight;
	size_t mOffsetX;
	size_t mOffsetY;
	T* mData;
	T mClearValue;
	bool mNeverClear;
};

typedef OutputChannel<Eigen::Vector3f> Output3D;
typedef OutputChannel<float> Output1D;
typedef OutputChannel<uint64> OutputCounter;
typedef OutputChannel<Spectrum> OutputSpectral;
}
