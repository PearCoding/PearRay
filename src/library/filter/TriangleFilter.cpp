#include "TriangleFilter.h"

namespace PR {
float TriangleFilter::evalWeight(float x, float y) const
{
	if (mRadius == 0)
		return 1;

	int ix = std::min((int)std::round(std::abs(x)), mRadius + 1);
	int iy = std::min((int)std::round(std::abs(y)), mRadius + 1);

	return mCache.at(iy * (mRadius + 1) + ix);
}

void TriangleFilter::cache()
{
	if (mRadius == 0)
		return;

	const int halfSize = mRadius + 1;
	mCache.resize(halfSize * (size_t)halfSize);

	float sum1 = 0.0f;
	float sum2 = 0.0f;
	float sum4 = 0.0f;
	for (int y = 0; y < halfSize; ++y) {
		for (int x = 0; x < halfSize; ++x) {
			const float r			 = std::sqrt(x * x + y * y);
			const float val			 = r <= mRadius ? 1 - r / (float)mRadius : 0.0f;
			mCache[y * halfSize + x] = val;

			if (y == 0 && x == 0)
				sum1 += val;
			else if (y == 0 || x == 0)
				sum2 += val;
			else
				sum4 += val;
		}
	}

	float norm = 1.0f / (sum1 + 2 * sum2 + 4 * sum4);
	for (int y = 0; y < halfSize; ++y)
		for (int x = 0; x < halfSize; ++x)
			mCache[y * halfSize + x] *= norm;
}
} // namespace PR