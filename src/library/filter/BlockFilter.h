#pragma once

#include "IFilter.h"

namespace PR {
class PR_LIB BlockFilter : public IFilter {
public:
	explicit BlockFilter(size_t radius = 1)
		: mRadius(radius)
	{
	}

	size_t radius() const override { return mRadius; }
	float evalWeight(float, float) const override { return 1.0f / ((2 * mRadius + 1) * (2 * mRadius + 1)); }

private:
	size_t mRadius;
};
} // namespace PR