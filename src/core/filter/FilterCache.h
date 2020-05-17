#pragma once

#include "IFilter.h"

namespace PR {
class PR_LIB_CORE FilterCache {
public:
	FilterCache(IFilter* filter)
		: mRadius(filter->radius())
		, mCache((2 * mRadius + 1) * (2 * mRadius + 1))
	{
	}

	inline int radius() const { return mRadius; }
	inline float evalWeight(float x, float y) const
	{
		int ix = std::max(std::min((int)std::round(x), mRadius), -mRadius) + mRadius;
		int iy = std::max(std::min((int)std::round(y), mRadius), -mRadius) + mRadius;

		return mCache.at(iy * (2 * mRadius + 1) + ix);
	}

private:
	int mRadius;
	std::vector<float> mCache;
};
} // namespace PR