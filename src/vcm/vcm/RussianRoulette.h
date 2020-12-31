#pragma once

#include "Random.h"

#include <optional>

namespace PR {
class RussianRoulette {
private:
	const size_t mMinRayDepth;
	const float mFactor;
	const bool mIgnoreDelta;

public:
	inline explicit RussianRoulette(size_t minRayDepth, float factor = 0.9f, bool ignoreDelta = true)
		: mMinRayDepth(minRayDepth)
		, mFactor(factor)
		, mIgnoreDelta(ignoreDelta)
	{
	}

	inline float probability(uint32 path_length, bool delta = false) const
	{
		if (path_length == 0 || (mIgnoreDelta && delta))
			return 1.0f;

		if (path_length >= mMinRayDepth) {
			constexpr float SCATTER_EPS = 1e-4f;
			const float scatProb		= std::min<float>(1.0f, std::pow(mFactor, path_length - mMinRayDepth));
			return scatProb <= SCATTER_EPS ? 0.0f : scatProb;
		} else {
			return 1.0f;
		}
	}

	inline std::optional<float> check(Random& rnd, uint32 path_length, bool delta = false) const
	{
		const float scatProb = probability(path_length, delta);
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