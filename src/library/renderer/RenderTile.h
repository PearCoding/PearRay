#pragma once

#include "PR_Config.h"

namespace PR
{
	class PR_LIB RenderTile
	{
	public:
		RenderTile(uint32 sx, uint32 sy, uint32 ex, uint32 ey);
		void inc();
		void reset();

		inline bool isWorking() const
		{
			return mWorking;
		}

		inline void setWorking(bool b)
		{
			mWorking = b;
		}

		inline uint32 sx() const
		{
			return mSX;
		}

		inline uint32 sy() const
		{
			return mSY;
		}

		inline uint32 ex() const
		{
			return mEX;
		}

		inline uint32 ey() const
		{
			return mEY;
		}

		inline uint32 samplesRendered() const
		{
			return mSamplesRendered;
		}
	private:
		bool mWorking;

		uint32 mSX;
		uint32 mSY;
		uint32 mEX;
		uint32 mEY;

		uint32 mSamplesRendered;
	};
}
