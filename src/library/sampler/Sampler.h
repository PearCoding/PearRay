#pragma once

#include "Random.h"

namespace PR
{
	class PR_LIB Sampler
	{
	public:
		virtual ~Sampler() {}

		virtual float generate1D(uint32 index) = 0;
		virtual PM::vec2 generate2D(uint32 index) = 0;
		virtual PM::vec3 generate3D(uint32 index) = 0;

	};
}
