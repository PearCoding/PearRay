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
	const size_t halfSize = mRadius + 1;
	mCache.resize(halfSize * halfSize);

	auto gauss = [](float x, float alpha) { return std::exp(-alpha * x * x); };

	const float dev2  = 0.2f;
	const float alpha = 1 / (2 * dev2);

	float sum = 0;
	for (size_t y = 0; y < halfSize; ++y) {
		for (size_t x = 0; x < halfSize; ++x) {
			mCache[y * halfSize + x] = gauss(y / (float)mRadius, alpha) * gauss(x / (float)mRadius, alpha);
			sum += mCache[y * halfSize + x];
		}
	}

	for (size_t y = 0; y < halfSize; ++y)
		for (size_t x = 0; x < halfSize; ++x)
			mCache[y * halfSize + x] /= sum;
}
} // namespace PR