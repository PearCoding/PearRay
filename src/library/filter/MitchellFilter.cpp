#include "MitchellFilter.h"

namespace PR {
float MitchellFilter::evalWeight(float x, float y) const
{
	if (mRadius == 0)
		return 1;

	size_t ix = std::min<size_t>(std::round(std::abs(x)), mRadius + 1);
	size_t iy = std::min<size_t>(std::round(std::abs(y)), mRadius + 1);

	return mCache.at(iy * (mRadius + 1) + ix);
}

// [-2,2]
static inline float mitchell(float x, float B, float C)
{
	x = std::abs(x);
	if (x < 1)
		return ((12 - 9 * B - 6 * C) * x * x * x + (-18 + 12 * B + 6 * C) * x * x + (6 - 2 * B)) / 6;
	else if (x < 2)
		return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x + (-12 * B - 48 * C) * x + (8 * B + 24 * C)) / 6;
	else
		return 0;
}
void MitchellFilter::cache()
{
	const size_t halfSize = mRadius + 1;
	mCache.resize(halfSize * halfSize);

	auto filter = [=](float x) { return mitchell(2 * x / (mRadius + 1), 1 / 3.0f, 1 / 3.0f); };

	float sum = 0;
	for (size_t y = 0; y < halfSize; ++y) {
		for (size_t x = 0; x < halfSize; ++x) {
			mCache[y * halfSize + x] = filter(y) * filter(x);
			sum += mCache[y * halfSize + x];
		}
	}

	for (size_t y = 0; y < halfSize; ++y)
		for (size_t x = 0; x < halfSize; ++x)
			mCache[y * halfSize + x] /= sum;
}
} // namespace PR