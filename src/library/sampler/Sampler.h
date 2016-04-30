#pragma once

#include "Config.h"
#include "PearMath.h"
#include "Random.h"

namespace PR
{
	class PR_LIB Sampler
	{
	public:
		virtual PM::vec generate(Random& rng) = 0;
	};
}