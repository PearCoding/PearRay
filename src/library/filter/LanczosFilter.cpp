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

// Using integer coordinates does not work well with lanczos filter. Everything except x=0 is zero...
inline static float sinc(float x) { return PR_1_PI * std::sin(PR_PI * x) / x; }

void LanczosFilter::cache()
{
	const size_t halfSize = mRadius + 1;
	mCache.resize(halfSize * halfSize);

	auto lanczos = [&](float x) { return sinc(x) * sinc(x / mRadius); };

	float sum = 0;
	for (size_t y = 0; y < halfSize; ++y) {
		const float fy = y == 0 ? 1 : lanczos(y);
		for (size_t x = 0; x < halfSize; ++x) {
			const float fx			 = x == 0 ? 1 : lanczos(x);
			mCache[y * halfSize + x] = fy * fx;
			sum += mCache[y * halfSize + x];
		}
	}

	for (size_t y = 0; y < halfSize; ++y)
		for (size_t x = 0; x < halfSize; ++x)
			mCache[y * halfSize + x] /= sum;
}
} // namespace PR