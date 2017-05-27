#pragma once

#include "Random.h"

namespace PR {
class PR_LIB Sampler {
public:
	virtual ~Sampler() {}

	virtual float generate1D(uint32 index)			 = 0;
	virtual Eigen::Vector2f generate2D(uint32 index) = 0;
	virtual Eigen::Vector3f generate3D(uint32 index) = 0;
};
}
