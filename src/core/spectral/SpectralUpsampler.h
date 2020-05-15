#pragma once

#include "spectral/ParametricBlob.h"
#include "spectral/SpectralBlob.h"

namespace PR {
// Base on Wenzel Jakob and Johannes Hanika. 2019. A Low-Dimensional Function Space for Efficient Spectral Upsampling.
// In Computer Graphics Forum (Proceedings of Eurographics) 38(2).
//
// Files with coefficents can be generated based on the paper and the tool found in the supplemental material
// (TODO) Embed at least the sRGB file
class PR_LIB_CORE SpectralUpsampler {
public:
	SpectralUpsampler(const std::wstring& filename);
	~SpectralUpsampler();

	void prepare(const float* r, const float* g, const float* b, float* out_a, float* out_b, float* out_c, size_t elems);
	static void compute(const float* a, const float* b, const float* c, const float* wavelengths, float* out_weights, size_t elems);
	static void computeSingle(float a, float b, float c, const float* wavelengths, float* out_weights, size_t elems);

	inline static SpectralBlob compute(const ParametricBlob& p, const SpectralBlob& in)
	{
		SpectralBlob blob;
		computeSingle(p(0), p(1), p(2), in.data(), blob.data(), PR_SPECTRAL_BLOB_SIZE);
		return blob;
	}

private:
	std::unique_ptr<struct _SpectralUpsamplerInternal> mInternal;
};
} // namespace PR