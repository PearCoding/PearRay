#pragma once

#include "Config.h"
#include "PearMath.h"
#include "Random.h"

namespace PR
{
	class PR_LIB Sampler
	{
	public:
		virtual ~Sampler() {}

		virtual float generate1D() = 0;
		virtual PM::vec2 generate2D() = 0;
		virtual PM::vec3 generate3D() = 0;

	};
}