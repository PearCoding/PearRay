
// IWYU pragma: private, include "sampler/Distribution1D.h"

namespace PR {

inline Distribution1D::Distribution1D(size_t size)
	: mValues(size)
	, mCDF(size + 1)
	, mIntegral(0.0f)
{
}

inline void Distribution1D::setup()
{
	mCDF[0]		   = 0.0f;
	const size_t n = size();
	for (size_t i = 1; i < n + 1; ++i)
		mCDF[i] = mCDF[i - 1] + mValues[i - 1] / n;

	mIntegral = mCDF[n];
	if (mIntegral < PR_EPSILON) {
		for (size_t i = 1; i < n + 1; ++i)
			mCDF[i] = float(i) / float(n);
	} else {
		for (size_t i = 1; i < n + 1; ++i)
			mCDF[i] /= mIntegral;
	}
}

inline float Distribution1D::sampleContinuous(float u, float& pdf, size_t* offset) const
{
	size_t off = Interval::binary_search(mCDF.size(), [&](int index) {
		return mCDF[index] <= u;
	});

	if (offset)
		*offset = off;

	float du = u - mCDF[off];
	float k	 = mCDF[off + 1] - mCDF[off];
	if (k > PR_EPSILON)
		du /= k;

	pdf = mValues[off] / mIntegral;

	return (off + du) / size();
}

inline float Distribution1D::sampleContinuous(float u, float& pdf, const float* cdf, size_t size)
{
	size_t off = Interval::binary_search(size, [&](int index) {
		return cdf[index] <= u;
	});

	float du = u - cdf[off];
	float k	 = cdf[off + 1] - cdf[off];
	if (k > PR_EPSILON)
		du /= k;

	pdf = k * (size - 1);

	return (off + du) / (size - 1);
}

inline float Distribution1D::continuousPdf(float u, size_t* offset) const
{
	size_t off = Interval::binary_search(mCDF.size(), [&](int index) {
		return mCDF[index] <= u;
	});
	if (offset)
		*offset = off;

	return mValues[off] / mIntegral;
}

inline float Distribution1D::continuousPdf(float u, const float* cdf, size_t size)
{
	size_t off = Interval::binary_search(size, [&](int index) {
		return cdf[index] <= u;
	});

	float k = cdf[off + 1] - cdf[off];
	return k * (size - 1);
}

inline size_t Distribution1D::sampleDiscrete(float u, float& pdf) const
{
	size_t off = Interval::binary_search(mCDF.size(), [&](int index) {
		return mCDF[index] <= u;
	});

	pdf = mValues[off] / (mIntegral * size());

	return off;
}

inline size_t Distribution1D::sampleDiscrete(float u, float& pdf, const float* cdf, size_t size)
{
	size_t off = Interval::binary_search(size, [&](int index) {
		return cdf[index] <= u;
	});

	const float value = off == 0 ? 0.0f : cdf[off + 1] - cdf[off];
	pdf				  = value / (size - 1);
	return off;
}

inline float Distribution1D::discretePdf(float u) const
{
	size_t off = Interval::binary_search(mCDF.size(), [&](int index) {
		return mCDF[index] <= u;
	});

	return mValues[off] / (mIntegral * size());
}
} // namespace PR
