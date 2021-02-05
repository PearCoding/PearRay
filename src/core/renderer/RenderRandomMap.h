#pragma once

#include "Random.h"

namespace PR {
class RenderContext;

/// A fullsized map of random generators. For each pixel each!
/// No mutex check. Make sure only one thread is accessing
class PR_LIB_CORE RenderRandomMap {
public:
	RenderRandomMap(RenderContext* context);
	~RenderRandomMap() = default;

	inline Random& random(const Point2i& globalP) { return mRandoms[globalP(0) + globalP(1) * mImageSize.Width]; }

private:
	const Size2i mImageSize;

	std::vector<Random> mRandoms; // For each pixel
};
} // namespace PR
