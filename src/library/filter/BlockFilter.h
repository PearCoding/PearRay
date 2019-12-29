#pragma once

#include "IFilter.h"

namespace PR {
class PR_LIB BlockFilter : public IFilter {
public:
	explicit BlockFilter(int radius = 1)
		: mRadius(radius)
	{
	}

	int radius() const override { return mRadius; }
	float evalWeight(float, float) const override { return 1.0f / ((2 * mRadius + 1) * (2 * mRadius + 1)); }

private:
	int mRadius;
};
} // namespace PR