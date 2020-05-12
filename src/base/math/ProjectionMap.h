#pragma once

#include "PR_Config.h"

namespace PR {
// http://www.keithschwarz.com/darts-dice-coins/
class PR_LIB_BASE ProjectionMap {
public:
	inline explicit ProjectionMap(uint32 res)
		: mProbability(nullptr)
		, mProbTable(nullptr)
		, mAliasTable(nullptr)
		, mResolution(res)
	{
		PR_ASSERT(res > 0, "resolution of ProjectionMap has to greater 0");

		mProbability = new float[res];
	}

	inline ~ProjectionMap()
	{
		delete[] mProbability;

		if (mProbTable)
			delete[] mProbTable;

		if (mAliasTable)
			delete[] mAliasTable;
	}

	inline void setProbability(uint32 x, float value)
	{
		PR_ASSERT(!mProbTable, "probability table has to be initialized before using it");
		mProbability[x] = value;
	}

	inline float probability(uint32 x) const
	{
		return mProbability[x];
	}

	inline void normalize()
	{
		float sum = 0;
		for (uint32 i = 0; i < mResolution; ++i)
			sum += mProbability[i];

		if (sum <= PR_EPSILON)
			return;

		sum = 1 / sum;
		scale(sum);
	}

	inline void rebound()
	{
		float max = 0;
		for (uint32 i = 0; i < mResolution; ++i)
			max = std::max(max, mProbability[i]);

		if (max <= PR_EPSILON)
			return;

		max = 1 / max;
		scale(max);
	}

	inline void scale(float f)
	{
		for (uint32 i = 0; i < mResolution; ++i)
			mProbability[i] *= f;
	}

	inline bool isSetup() const
	{
		return mProbTable != nullptr;
	}

	inline void setup()
	{
		// Setup tables
		mProbTable  = new float[mResolution];
		mAliasTable = new uint32[mResolution];
		std::deque<uint32> small;
		std::deque<uint32> large;

		float* prob = new float[mResolution]; // Temporary ----
		for (uint32 i = 0; i < mResolution; ++i) {
			float p = mProbability[i] * mResolution;
			if (p < 1)
				small.push_back(i);
			else
				large.push_back(i);

			prob[i] = p;
		}

		while (!small.empty() && !large.empty()) {
			uint32 s = small.front();
			uint32 l = large.front();

			small.pop_front();
			large.pop_front();

			mProbTable[s]  = prob[s];
			mAliasTable[s] = l;

			prob[l] = (prob[l] + prob[s]) - 1;
			if (prob[l] < 1)
				small.push_back(l);
			else
				large.push_back(l);
		}
		delete[] prob; // Not needed anymore ----

		while (!large.empty()) {
			uint32 l = large.front();
			large.pop_front();
			mProbTable[l]  = 1;
			mAliasTable[l] = l; // Not really necessary, but safe is safe.
		}

		while (!small.empty()) {
			uint32 l = small.front();
			small.pop_front();
			mProbTable[l]  = 1;
			mAliasTable[l] = l;
		}

		// Remap from [0,n] to [0,1]
		for (uint32 i = 0; i < mResolution; ++i)
			mProbTable[i] /= mResolution;
	}

	// u1, u2 in [0, 1)
	inline uint32 sample(float u1, float u2, float& pdf) const
	{
		uint32 i = std::min(std::max<uint32>(u1 * mResolution, 0), mResolution - 1);
		if (u2 < mProbTable[i]) {
			pdf = mProbability[i];
			return i;
		} else {
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
