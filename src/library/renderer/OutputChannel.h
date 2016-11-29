#pragma once

#include "renderer/Renderer.h"

namespace PR
{
	class Renderer;

	template<typename T>
	class PR_LIB OutputChannel
	{
		PR_CLASS_NON_COPYABLE(OutputChannel);
	public:
		inline OutputChannel(Renderer* renderer) :
			mRenderer(renderer), mData(nullptr),
			mRW(0), mRH(0), mCX(0), mCY(0)
		{
		}

		inline ~OutputChannel()
		{
			deinit();
		}
		
		inline void init()
		{
			PR_ASSERT(!mData);
			mRW = mRenderer->renderWidth();
			mRH = mRenderer->renderHeight();
			mCX = mRenderer->cropPixelOffsetX();
			mCY = mRenderer->cropPixelOffsetY();
			mData = new T[mRW * mRH];
		}

		inline void deinit()
		{
			if(mData)
			{
				delete[] mData;
				mData = nullptr;
			}
		}

		void clear()
		{
			if(!mData)
				return;

			std::memset(mData, 0, mRW*mRH*sizeof(T));
		}

		inline void pushFragment(uint32 x, uint32 y, const T& v)
		{
			PR_DEBUG_ASSERT(x >= mCX && x < mCX + mRW);
			PR_DEBUG_ASSERT(y >= mCY && y < mCY + mRH);
			mData[(y-mCY)*mRW+(x-mCX)] = v;
		}

		inline T getFragment(uint32 x, uint32 y) const
		{
			PR_DEBUG_ASSERT(x >= mCX && x < mCX + mRW);
			PR_DEBUG_ASSERT(y >= mCY && y < mCY + mRH);
			return mData[(y-mCY)*mRW+(x-mCX)];
		}

		inline void pushFragmentBounded(uint32 x, uint32 y, const T& v)
		{
			mData[y*mRW+x] = v;
		}

		inline T getFragmentBounded(uint32 x, uint32 y) const
		{
			return mData[y*mRW+x];
		}
		
		inline T* ptr() const
		{
			return mData;
		}

		inline void fill(const T& v)
		{
			std::fill_n(mData, mRW * mRH, v);
		}

	private:
		Renderer* mRenderer;
		T* mData;

		// Temp
		uint32 mRW;
		uint32 mRH;
		uint32 mCX;
		uint32 mCY;
	};

	typedef OutputChannel<PM::avec3> Output3D;
	typedef OutputChannel<float> Output1D;
	typedef OutputChannel<Spectrum> OutputSpectral;
}