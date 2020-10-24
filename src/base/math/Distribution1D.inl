
// IWYU pragma: private, include "sampler/Distribution1D.h"

namespace PR {

inline Distribution1D::Distribution1D(size_t size)
	: mCDF(size + 1)
{
}

template <typename Func>
inline void Distribution1D::generate(Func func, float* integral)
{
	mCDF[0]		   = 0.0f;
	const size_t n = numberOfValues();
	for (size_t i = 0; i < n; ++i)
		mCDF[i + 1] = mCDF[i] + func(i);

	const float intr = mCDF[n];
	if (integral)
		*integral = intr;

	if (intr <= PR_EPSILON) {
		for (size_t i = 1; i < n + 1; ++i)
			mCDF[i] = float(i) / float(n);
	} else {
		for (size_t i = 1; i < n + 1; ++i)
			mCDF[i] /= intr;
	}
}

inline float Distribution1D::sampleContinuous(float u, float& pdf, size_t* offset) const
{
	return sampleContinuous(u, pdf, mCDF.data(), mCDF.size(), offset);
}

inline float Distribution1D::sampleContinuous(float u, float& pdf, const float* cdf, size_t size, size_t* offset)
{
	float rem;
	const size_t off = sampleDiscrete(u, pdf, cdf, size, &rem);

	if (offset)
		*offset = off;

	pdf *= (size - 1);
	return (off + rem) / (size - 1);
}

inline float Distribution1D::continuousPdf(float x, size_t* offset) const
{
	return continuousPdf(x, mCDF.data(), mCDF.size(), offset);
}

inline float Distribution1D::continuousPdf(float x, const float* cdf, size_t size, size_t* offset)
{
	const size_t off = std::min<size_t>(size - 2, x * (size - 1));
	if (offset)
		*offset = off;
	return discretePdf(off, cdf, size) * (size - 1);
}

inline float Distribution1D::evalContinuous(float u) const
{
	return evalContinuous(u, mCDF.data(), mCDF.size());
}

inline float Distribution1D::evalContinuous(float x, const float* cdf, size_t size)
{
	const size_t off = std::min<size_t>(size - 2, x * (size - 1));
	const float dt	 = x * (size - 1) - off;
	return cdf[off] * (1 - dt) + cdf[off + 1] * dt;
}

inline size_t Distribution1D::sampleDiscrete(float u, float& pdf, float* remainder) const
{
	return sampleDiscrete(u, pdf, mCDF.data(), mCDF.size(), remainder);
}

inline size_t Distribution1D::sampleDiscrete(float u, float& pdf, const float* cdf, size_t size, float* remainder)
{
	const size_t off = Interval::binary_search(size, [&](int index) {
		return cdf[index] <= u;
	});

	if (remainder) {
		*remainder	  = u - cdf[off];
		const float k = cdf[off + 1] - cdf[off];
		if (k > PR_EPSILON)
			*remainder /= k;
	}

	pdf = discretePdf(off, cdf, size);
	return off;
}

inline float Distribution1D::discretePdf(size_t x) const
{
	return discretePdf(x, mCDF.data(), mCDF.size());
}

inline float Distribution1D::discretePdf(size_t x, const float* cdf, size_t size)
{
	PR_UNUSED(size);
	PR_ASSERT(x < size - 1, "Expected x to be of correct range");
	return cdf[x + 1] - cdf[x];
}

inline float Distribution1D::evalDiscrete(float u) const
{
	return evalDiscrete(u, mCDF.data(), mCDF.size());
}

inline float Distribution1D::evalDiscrete(float x, const float* cdf, size_t size)
{
	const size_t off = std::min<size_t>(size - 2, x * (size - 1));
	return cdf[off];
}

inline void Distribution1D::reducePDFBy(float v, float* integral)
{
	// Extract
	std::vector<float> pdfs;
	pdfs.resize(mCDF.size() - 1, 0.0f);
	for (size_t i = 0; i < mCDF.size() - 1; ++i)
		pdfs[i] = discretePdf(i);

	// Reduce
	for (float& p : pdfs)
		p = std::max(0.0f, p - v);

	// Rebuild
	generate([&](size_t i) { return pdfs[i]; }, integral);
}

////////////////////////////// StaticCDF

template <size_t N>
inline float Distribution1D::sampleContinuous(float u, float& pdf, const StaticCDF<N>& cdf)
{
	return sampleContinuous(u, pdf, cdf.data(), N + 1);
}

template <size_t N>
inline float Distribution1D::evalContinuous(float u, const StaticCDF<N>& cdf)
{
	return evalContinuous(u, cdf.data(), N + 1);
}

template <size_t N>
inline float Distribution1D::continuousPdf(float x, const StaticCDF<N>& cdf)
{
	return continuousPdf(x, cdf.data(), N + 1);
}

template <size_t N>
inline size_t Distribution1D::sampleDiscrete(float u, float& pdf, const StaticCDF<N>& cdf)
{
	return sampleDiscrete(u, pdf, cdf.data(), N + 1);
}

template <size_t N>
inline float Distribution1D::discretePdf(size_t x, const StaticCDF<N>& cdf)
{
	return discretePdf(x, cdf.data(), N + 1);
}

template <size_t N>
inline float Distribution1D::evalDiscrete(float u, const StaticCDF<N>& cdf)
{
	return evalDiscrete(u, cdf.data(), N + 1);
}
} // namespace PR
