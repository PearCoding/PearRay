#pragma once

#include "renderer/RenderContext.h"

namespace PR
{
	class RenderContext;

	template<typename T>
	class PR_LIB OutputChannel
	{
		PR_CLASS_NON_COPYABLE(OutputChannel);
	public:
		inline OutputChannel(RenderContext* renderer) :
			mRenderer(renderer), mData(nullptr)
		{
		}

		inline ~OutputChannel()
		{
			deinit();
		}
		
		inline void init()
		{
			PR_ASSERT(!mData);
			mData = new T[mRenderer->width() * mRenderer->height()];
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

			std::memset(mData, 0, mRenderer->width() * mRenderer->height() * sizeof(T));
		}

		inline void pushFragment(uint32 x, uint32 y, const T& v) const
		{
			PR_DEBUG_ASSERT(x >= mRenderer->offsetX() && x < mRenderer->offsetX() + mRenderer->width());
			PR_DEBUG_ASSERT(y >= mRenderer->offsetY() && y < mRenderer->offsetY() + mRenderer->height());
			mData[(y-mRenderer->offsetY())*mRenderer->width()+(x-mRenderer->offsetX())] = v;
		}

		inline const T& getFragment(uint32 x, uint32 y) const
		{
			PR_DEBUG_ASSERT(x >= mRenderer->offsetX() && x < mRenderer->offsetX() + mRenderer->width());
			PR_DEBUG_ASSERT(y >= mRenderer->offsetY() && y < mRenderer->offsetY() + mRenderer->height());
			return mData[(y-mRenderer->offsetY())*mRenderer->width()+(x-mRenderer->offsetX())];
		}

		inline void pushFragmentBounded(uint32 x, uint32 y, const T& v)
		{
			PR_DEBUG_ASSERT(x < mRenderer->width());
			PR_DEBUG_ASSERT(y < mRenderer->height());
			mData[y*mRenderer->width()+x] = v;
		}

		inline const T& getFragmentBounded(uint32 x, uint32 y) const
		{
			PR_DEBUG_ASSERT(x < mRenderer->width());
			PR_DEBUG_ASSERT(y < mRenderer->height());
			return mData[y*mRenderer->width()+x];
		}
		
		inline T* ptr() const
		{
			return mData;
		}

		inline void fill(const T& v)
		{
			std::fill_n(mData, mRenderer->width() * mRenderer->height(), v);
		}

	private:
		RenderContext* mRenderer;
		T* mData;
	};

	typedef OutputChannel<PM::avec3> Output3D;
	typedef OutputChannel<float> Output1D;
	typedef OutputChannel<Spectrum> OutputSpectral;
}