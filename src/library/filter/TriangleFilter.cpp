#include "TriangleFilter.h"

namespace PR {
float TriangleFilter::evalWeight(float x, float y) const
{
	if (mRadius == 0)
		return 1;

	size_t ix = std::min<size_t>(std::round(std::abs(x)), mRadius + 1);
	size_t iy = std::min<size_t>(std::round(std::abs(y)), mRadius + 1);

	return mCache.at(iy * (mRadius + 1) + ix);
}

void TriangleFilter::cache()
{
	if (mRadius == 0)
		return;

	const size_t halfSize = mRadius + 1;
	mCache.resize(halfSize * halfSize);

	for (size_t y = 0; y < halfSize; ++y) {
		for (size_t x = 0; x < halfSize; ++x) {
			const float r			 = std::sqrt(x * x + y * y);
			mCache[y * halfSize + x] = r <= mRadius ? 1 - r / (float)mRadius : 0.0f;
		}
	}

	float norm = 1.0f / (PR_PI * mRadius);
	for (size_t y = 0; y < halfSize; ++y)
		for (size_t x = 0; x < halfSize; ++x)
			mCache[y * halfSize + x] *= norm;
}
} // namespace PR