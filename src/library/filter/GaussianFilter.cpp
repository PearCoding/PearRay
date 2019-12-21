#include "GaussianFilter.h"

namespace PR {
float GaussianFilter::evalWeight(float x, float y) const
{
	if (mRadius == 0)
		return 1;

	size_t ix = std::min<size_t>(std::round(std::abs(x)), mRadius + 1);
	size_t iy = std::min<size_t>(std::round(std::abs(y)), mRadius + 1);

	return mCache.at(iy * (mRadius + 1) + ix);
}

void GaussianFilter::cache()
{
	if (mRadius == 0)
		return;

	const size_t halfSize = mRadius + 1;
	mCache.resize(halfSize * halfSize);

	auto gauss = [](float x, float alpha) {
		return x <= 1.0f ? std::exp(-alpha * x * x) : 0.0f;
	};

	const float dev2  = 0.2f;
	const float alpha = 1 / (2 * dev2);

	float sum1 = 0.0f;
	float sum2 = 0.0f;
	float sum4 = 0.0f;
	for (size_t y = 0; y < halfSize; ++y) {
		for (size_t x = 0; x < halfSize; ++x) {
			const float r			 = std::sqrt(x * x + y * y) / (float)mRadius;
			const float val			 = gauss(r, alpha);
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