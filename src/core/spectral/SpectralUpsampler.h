#pragma once

#include "spectral/ParametricBlob.h"
#include "spectral/SpectralBlob.h"

namespace PR {
class Serializer;

// Base on Wenzel Jakob and Johannes Hanika. 2019. A Low-Dimensional Function Space for Efficient Spectral Upsampling.
// In Computer Graphics Forum (Proceedings of Eurographics) 38(2).
//
// Files with coefficents can be generated based on the paper and the tool found in the supplemental material
// A special case is rgb (0,0,0) which was not handled in the original paper. We set a,b=0 and c=-50 to approximate a zero spectrum
class PR_LIB_CORE SpectralUpsampler {
public:
	explicit SpectralUpsampler(Serializer& serializer);
	~SpectralUpsampler();

	void prepare(const float* PR_RESTRICT r, const float* PR_RESTRICT g, const float* PR_RESTRICT b,
				 float* PR_RESTRICT out_a, float* PR_RESTRICT out_b, float* PR_RESTRICT out_c, size_t elems);
	void prepare(const float* PR_RESTRICT rgb, float* PR_RESTRICT out, size_t elems);

	inline static void compute(const float* PR_RESTRICT a, const float* PR_RESTRICT b, const float* PR_RESTRICT c,
							   const float* PR_RESTRICT wavelengths, float* PR_RESTRICT out_weights, size_t elems)
	{
		PR_OPT_LOOP
		for (size_t i = 0; i < elems; ++i) {
			const float x  = std::fma(std::fma(a[i], wavelengths[i], b[i]), wavelengths[i], c[i]);
			const float y  = 1.0f / std::sqrt(std::fma(x, x, 1.0f));
			out_weights[i] = std::fma(0.5f * x, y, 0.5f);
		}
	}

	inline static void computeSingle(float a, float b, float c,
									 const float* PR_RESTRICT wavelengths, float* PR_RESTRICT out_weights, size_t elems)
	{
		PR_OPT_LOOP
		for (size_t i = 0; i < elems; ++i) {
			const float x  = std::fma(std::fma(a, wavelengths[i], b), wavelengths[i], c);
			const float y  = 1.0f / std::sqrt(std::fma(x, x, 1.0f));
			out_weights[i] = std::fma(0.5f * x, y, 0.5f);
		}
	}

	inline static SpectralBlob compute(const ParametricBlob& p, const SpectralBlob& wvls)
	{
		const SpectralBlob x = (p(0) * wvls + p(1)) * wvls + p(2);
		return 0.5f * x * (x.square() + 1.0f).rsqrt() + 0.5f;
	}

private:
	std::unique_ptr<struct _SpectralUpsamplerInternal> mInternal;
};
} // namespace PR