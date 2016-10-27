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

		void clear(uint32 sx = 0, uint32 sy = 0, uint32 ex = 0, uint32 ey = 0)
		{
			if(!mData)
				return;
			
			const auto w = mRenderer->width();
			if(sx == 0 && sy == 0 &&
				(ex == 0 || ex == w) &&
				(ey == 0 || ey == mRenderer->height()))// Full clear
			{
				std::memset(mData, 0, w*mRenderer->height()*sizeof(T));
			}
			else
			{
				PR_ASSERT(sx < ex);
				PR_ASSERT(sy < ey);

				for(uint32 y = sy; y < ey; ++y)// Line by line
					std::memset(&mData[y*w + sx], 0, (ex - sx)* sizeof(T));
			}
		}

		inline void pushFragment(uint32 x, uint32 y, const T& v)
		{
			mData[y*mRenderer->width()+x] = v;
		}

		inline T getFragment(uint32 x, uint32 y) const
		{
			return mData[y*mRenderer->width()+x];
		}
		
		inline T* ptr() const
		{
			return mData;
		}

	private:
		Renderer* mRenderer;
		T* mData;
	};

	typedef OutputChannel<PM::avec3> Output3D;
	typedef OutputChannel<float> Output1D;
	typedef OutputChannel<Spectrum> OutputSpectral;
}