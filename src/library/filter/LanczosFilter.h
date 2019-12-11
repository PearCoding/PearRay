#pragma once

#include "IFilter.h"
#include <vector>

namespace PR {
class PR_LIB LanczosFilter : public IFilter {
public:
	explicit LanczosFilter(size_t radius = 1)
		: mRadius(radius)
	{
		cache();
	}

	size_t radius() const override { return mRadius; }
	float evalWeight(float, float) const override;

private:
	void cache();

	size_t mRadius;
	std::vector<float> mCache;
};
} // namespace PR