#pragma once

#include "Random.h"

#include <vector>

namespace PR {
/* An array containing uniform samples [0,1) */
class PR_LIB_CORE SampleArray {
public:
	inline SampleArray(size_t size, Random& rnd)
		: mValues(size)
		, mIt(0)
	{
		reset(rnd);
	}

	inline explicit SampleArray(size_t size)
		: mValues(size)
		, mIt(0)
	{
	}

	inline float next1D()
	{
		PR_ASSERT(mIt < mValues.size(), "Out of sample array bound");
		return mValues[mIt++];
	}

	inline Vector2f next2D()
	{
		return Vector2f(next1D(), next1D());
	}

	inline Vector3f next3D()
	{
		return Vector3f(next1D(), next1D(), next1D());
	}

	inline float& operator[](size_t ind)
	{
		PR_ASSERT(ind < mValues.size(), "Out of sample array bound");
		return mValues[ind];
	}

	inline float operator[](size_t ind) const
	{
		PR_ASSERT(ind < mValues.size(), "Out of sample array bound");
		return mValues[ind];
	}

	inline size_t size() const { return mValues.size(); }
	inline size_t currentPos() const { return mIt; }
	inline void reset() { mIt = 0; }
	inline void reset(Random& rnd)
	{
		mIt = 0;
		PR_OPT_LOOP
		for (size_t i = 0; i < mValues.size(); ++i)
			mValues[i] = rnd.getFloat();
	}

private:
	std::vector<float> mValues;
	size_t mIt;
};
} // namespace PR
