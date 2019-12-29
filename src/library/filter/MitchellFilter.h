#pragma once

#include "IFilter.h"
#include <vector>

namespace PR {
class PR_LIB MitchellFilter : public IFilter {
public:
	explicit MitchellFilter(int radius = 1)
		: mRadius(radius)
	{
		cache();
	}

	int radius() const override { return mRadius; }
	float evalWeight(float, float) const override;

private:
	void cache();

	int mRadius;
	std::vector<float> mCache;
};
} // namespace PR