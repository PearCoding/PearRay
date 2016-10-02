#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class PR_LIB ProjectionMap
	{
	public:
		inline ProjectionMap(uint32 res) :
			mProbability(nullptr), mProbTable(nullptr), mAliasTable(nullptr), mResolution(res)
		{
			PR_ASSERT(res > 0);

			mProbability = new float[res];
		}

		inline ~ProjectionMap()
		{
			delete[] mProbability;

			if(mProbTable)
				delete[] mProbTable;
			
			if(mAliasTable)
				delete[] mAliasTable;
		}

		inline void setProbability(uint32 x, float value)
		{
			PR_ASSERT(!mProbTable);
			mProbability[x] = value;
		}

		inline float probability(uint32 x) const
		{
			return mProbability[x];
		}

		inline bool isSetup() const
		{
			return mProbTable != nullptr;
		}

		inline void setup()
		{
			// Normalize
			float sum = 0;
			for(uint32 i = 0; i < mResolution; ++i)
				sum += mProbability[i];
			
			if(sum <= PM_EPSILON)
				return;

			sum = 1/sum;
			for(uint32 i = 0; i < mResolution; ++i)
				mProbability[i] *= sum;
			
			// Setup dice table
			float* prob = new float[mResolution];
			mProbTable = new float[mResolution];
			mAliasTable = new uint32[mResolution];

			std::deque<uint32> small;
			std::deque<uint32> large;

			for(uint32 i = 0; i < mResolution; ++i)
			{
				float p = mProbability[i]*mResolution;
				if(p < 1)
					small.push_back(i);
				else
					large.push_back(i);
				
				prob[i] = p;
			}

			while(!small.empty() && !large.empty())
			{
				uint32 s = small.front();
				uint32 l = large.front();

				small.pop_front();
				large.pop_front();

				mProbTable[s] = prob[s];
				mAliasTable[s] = l;

				prob[l] = (prob[l] + prob[s]) - 1;
				if(prob[l] < 1)
					small.push_back(l);
				else
					large.push_back(l);
			}

			while(!large.empty())
			{
				uint32 l = large.front();
				large.pop_front();
				mProbTable[l] = 1;
			}

			while(!small.empty())
			{
				uint32 l = small.front();
				small.pop_front();
				mProbTable[l] = 1;
			}

			// Remap from [0,n] to [0,1]
			for(uint32 i = 0; i < mResolution; ++i)
				mProbTable[i] /= mResolution;

			delete[] prob;
		}

		// u1, u2 in [0, 1]
		inline uint32 sample(float u1, float u2, float& pdf) const
		{
			uint32 i = (uint32)(u1*mResolution);
			if(mProbTable[i] < u2)
			{
				pdf = mProbTable[i];
				return i;
			}
			else
			{
				pdf = mProbability[mAliasTable[i]];
				return mAliasTable[i];
			}
		}

		inline uint32 resolution() const
		{
			return mResolution;
		}

	private:
		float* mProbability;
		// Alias method
		float* mProbTable;
		uint32* mAliasTable;
		uint32 mResolution;
	};
}