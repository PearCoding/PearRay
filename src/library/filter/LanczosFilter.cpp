#include "LanczosFilter.h"

namespace PR {
float LanczosFilter::evalWeight(float x, float y) const
{
	if (mRadius == 0)
		return 1;

	size_t ix = std::min<size_t>(std::round(std::abs(x)), mRadius + 1);
	size_t iy = std::min<size_t>(std::round(std::abs(y)), mRadius + 1);

	return mCache.at(iy * (mRadius + 1) + ix);
}

inline static float sinc(float x) { return PR_1_PI * std::sin(PR_PI * x) / x; }

void LanczosFilter::cache()
{
	const size_t halfSize = mRadius + 1;
	mCache.resize(halfSize * halfSize);

	auto lanczos = [&](float x) { return x <= PR_EPSILON
											 ? 1.0f
											 : (x <= mRadius
													? sinc(x) * sinc(x / mRadius)
													: 0.0f); };

	float sum1 = 0.0f;
	float sum2 = 0.0f;
	float sum4 = 0.0f;
	for (size_t y = 0; y < halfSize; ++y) {
		for (size_t x = 0; x < halfSize; ++x) {
			const float r			 = std::sqrt(x * x + y * y);
			const float val			 = lanczos(r);
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
	for (size_t y = 0; y < halfSize; ++y)
		for (size_t x = 0; x < halfSize; ++x)
			mCache[y * halfSize + x] *= norm;
}
} // namespace PR