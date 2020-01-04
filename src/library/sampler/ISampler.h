#pragma once

#include "Random.h"

namespace PR {
class PR_LIB ISampler {
public:
	inline explicit ISampler(uint32 max_samples)
		: mMaxSamples(max_samples)
	{
	}

	virtual ~ISampler() = default;

	virtual float generate1D(uint32 index)	= 0;
	virtual Vector2f generate2D(uint32 index) = 0;

	inline uint32 maxSamples() const { return mMaxSamples; }

private:
	const uint32 mMaxSamples;
};
} // namespace PR
