#pragma once

#include "IFilter.h"

namespace PR {
class PR_LIB_CORE FilterCache {
public:
	FilterCache(IFilter* filter)
		: mRadius(filter->radius())
		, mDiameter(static_cast<size_t>(2 * mRadius + 1))
		, mCache(mDiameter * mDiameter)
	{
		for (int y = -mRadius; y <= mRadius; ++y)
			for (int x = -mRadius; x <= mRadius; ++x)
				mCache[(y + mRadius) * mDiameter + x + mRadius] = filter->evalWeight(x, y);
	}

	inline int radius() const { return mRadius; }
	inline float evalWeight(float x, float y) const
	{
		int ix = std::max(std::min((int)std::round(x), mRadius), -mRadius) + mRadius;
		int iy = std::max(std::min((int)std::round(y), mRadius), -mRadius) + mRadius;

		return mCache.at(iy * mDiameter + ix);
	}

private:
	const int mRadius;
	const size_t mDiameter;
	std::vector<float> mCache;
};
} // namespace PR