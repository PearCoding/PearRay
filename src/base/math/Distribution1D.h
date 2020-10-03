#pragma once

#include "container/Interval.h"

#include <vector>
namespace PR {
// Compile-Time CDF constructor
template <size_t SampleCount>
class PR_LIB_BASE StaticCDF {
public:
	constexpr StaticCDF(const float* data)
	{
		mData[0] = 0.0f;
		for (size_t i = 1; i < SampleCount + 1; ++i)
			mData[i] = mData[i - 1] + data[i - 1] / SampleCount;

		if (mData[SampleCount] < PR_EPSILON) {
			for (size_t i = 1; i < SampleCount + 1; ++i)
				mData[i] = float(i) / float(SampleCount);
		} else {
			for (size_t i = 1; i < SampleCount + 1; ++i)
				mData[i] /= mData[SampleCount];
		}
	}

	// Sum
	constexpr StaticCDF(const float* data1, const float* data2, const float* data3)
	{
		mData[0] = 0.0f;
		for (size_t i = 1; i < SampleCount + 1; ++i)
			mData[i] = mData[i - 1] + (data1[i - 1] + data2[i - 1] + data3[i - 1]) / SampleCount;

		if (mData[SampleCount] < PR_EPSILON) {
			for (size_t i = 1; i < SampleCount + 1; ++i)
				mData[i] = float(i) / float(SampleCount);
		} else {
			for (size_t i = 1; i < SampleCount + 1; ++i)
				mData[i] /= mData[SampleCount];
		}
	}

	constexpr const float* data() const { return mData.data(); }
	constexpr size_t size() const { return SampleCount; }
	constexpr float at(size_t i) const { return mData[i]; }

private:
	std::array<float, SampleCount + 1> mData;
};

/* Piecewise 1D distribution */
class PR_LIB_BASE Distribution1D {
public:
	inline explicit Distribution1D(size_t numberOfValues);
	inline size_t numberOfValues() const { return mCDF.size() - 1; }
	inline size_t numberOfEntries() const { return mCDF.size(); }

	template <typename Func>
	inline void generate(Func func, float* integral = nullptr);

	inline float sampleContinuous(float u, float& pdf, size_t* offset = nullptr) const;
	inline float continuousPdf(float x, size_t* offset = nullptr) const;
	inline float evalContinuous(float u) const;

	inline size_t sampleDiscrete(float u, float& pdf, float* remainder = nullptr) const;
	inline float discretePdf(size_t x) const;
	inline float evalDiscrete(float u) const;

	// The given CDF is assumed to be normalized such that its last entry is exactly 1
	inline static float sampleContinuous(float u, float& pdf, const float* cdf, size_t size, size_t* offset = nullptr);
	inline static float evalContinuous(float u, const float* cdf, size_t size);
	inline static float continuousPdf(float u, const float* cdf, size_t size, size_t* offset = nullptr);

	inline static size_t sampleDiscrete(float u, float& pdf, const float* cdf, size_t size, float* remainder = nullptr);
	inline static float evalDiscrete(float u, const float* cdf, size_t size);
	inline static float discretePdf(size_t x, const float* cdf, size_t size);

	template <size_t N>
	inline static float sampleContinuous(float u, float& pdf, const StaticCDF<N>& cdf);
	template <size_t N>
	inline static float evalContinuous(float u, const StaticCDF<N>& cdf);
	template <size_t N>
	inline static float continuousPdf(float u, const StaticCDF<N>& cdf);
	template <size_t N>
	inline static size_t sampleDiscrete(float u, float& pdf, const StaticCDF<N>& cdf);
	template <size_t N>
	inline static float evalDiscrete(float u, const StaticCDF<N>& cdf);
	template <size_t N>
	inline static float discretePdf(size_t x, const StaticCDF<N>& cdf);

private:
	std::vector<float> mCDF;
};
} // namespace PR

#include "Distribution1D.inl"