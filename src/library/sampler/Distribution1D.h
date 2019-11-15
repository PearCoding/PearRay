#pragma once

#include "PR_Config.h"

#include <vector>
namespace PR {
/* Piecewise 1D distribution */
class PR_LIB Distribution1D {
public:
	Distribution1D(size_t size);

	inline size_t size() const { return mValues.size(); }
	inline float integral() const { return mIntegral; }

	template <typename Func>
	inline void generate(Func func)
	{
		for (size_t i = 0; i < size(); ++i)
			mValues[i] = func(i);

		setup();
	}

	float sampleContinuous(float u, float& pdf, size_t* offset = nullptr) const;
	float continuousPdf(float u, size_t* offset = nullptr) const;

	size_t sampleDiscrete(float u, float& pdf) const;
	float discretePdf(float u) const;

private:
	void setup();

	std::vector<float> mValues;
	std::vector<float> mCDF;
	float mIntegral;
};
} // namespace PR
