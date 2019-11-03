#pragma once

#include "Random.h"

namespace PR {
class PR_LIB Sampler {
public:
	virtual ~Sampler() = default;

	virtual float generate1D(uint32 index)			 = 0;
	virtual Vector2f generate2D(uint32 index) = 0;
};
} // namespace PR
