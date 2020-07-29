#pragma once

#include "container/Interval.h"

#include <vector>
namespace PR {
/* Piecewise 1D distribution */
class PR_LIB_CORE Distribution1D {
public:
	inline explicit Distribution1D(size_t size);

	inline size_t size() const { return mValues.size(); }
	inline float integral() const { return mIntegral; }

	template <typename Func>
	inline void generate(Func func)
	{
		for (size_t i = 0; i < size(); ++i)
			mValues[i] = func(i);

		setup();
	}

	inline float sampleContinuous(float u, float& pdf, size_t* offset = nullptr) const;
	inline float continuousPdf(float u, size_t* offset = nullptr) const;

	inline size_t sampleDiscrete(float u, float& pdf) const;
	inline float discretePdf(float u) const;

	// The given CDF is assumed to be normalized such that its last entry is exactly 1
	inline static float sampleContinuous(float u, float& pdf, const float* cdf, size_t size);
	inline static float continuousPdf(float u, const float* cdf, size_t size);
	inline static size_t sampleDiscrete(float u, float& pdf, const float* cdf, size_t size);

private:
	inline void setup();

	std::vector<float> mValues;
	std::vector<float> mCDF;
	float mIntegral;
};
} // namespace PR

#include "Distribution1D.inl"