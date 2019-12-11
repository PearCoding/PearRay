#pragma once

#include "IFilter.h"

namespace PR {
class PR_LIB TriangleFilter : public IFilter {
public:
	explicit TriangleFilter(size_t radius = 1)
		: mRadius(radius)
	{
	}

	size_t radius() const override { return mRadius; }
	float evalWeight(float, float) const;

private:
	size_t mRadius;
};
} // namespace PR