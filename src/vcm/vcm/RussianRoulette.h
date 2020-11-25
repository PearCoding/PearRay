#pragma once

#include "Random.h"

#include <optional>

namespace PR {
class RussianRoulette {
private:
	const size_t mMinRayDepth;
	const float mFactor;

public:
	inline explicit RussianRoulette(size_t minRayDepth, float factor = 0.9f)
		: mMinRayDepth(minRayDepth)
		, mFactor(factor)
	{
	}

	inline float probability(uint32 path_length) const
	{
		if (path_length == 0)
			return 1.0f;

		if (path_length >= mMinRayDepth) {
			constexpr float SCATTER_EPS = 1e-4f;
			const float scatProb		= std::min<float>(1.0f, std::pow(mFactor, path_length - mMinRayDepth));
			return scatProb <= SCATTER_EPS ? 0.0f : scatProb;
		} else {
			return 1.0f;
		}
	}

	inline std::optional<float> check(Random& rnd, uint32 path_length) const
	{
		const float scatProb = probability(path_length);
		if (scatProb <= PR_EPSILON) {
			return {};
		} else if (scatProb < 1.0f) {
			const float russian_prob = rnd.getFloat();
			if (russian_prob > scatProb)
				return {};
		}

		return { scatProb };
	}
};

} // namespace PR