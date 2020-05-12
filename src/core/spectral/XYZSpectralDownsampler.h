#pragma once

#include "PR_Config.h"

namespace PR {
// Base on Wenzel Jakob and Johannes Hanika. 2019. A Low-Dimensional Function Space for Efficient Spectral Upsampling.
// In Computer Graphics Forum (Proceedings of Eurographics) 38(2).
//
// Files with coefficents can be generated based on the paper and the tool found in the supplemental material
// (TODO) Embed the sRGB file
class PR_LIB_CORE SpectralMapper {
public:
	SpectralMapper(const std::wstring& filename);
	~SpectralMapper();

	void prepare(const float* r, const float* g, const float* b, float* out_a, float* out_b, float* out_c, size_t elems);
	static void compute(const float* a, const float* b, const float* c, const float* wavelengths, float* out_weights, size_t elems);

	static void toXYZ(const float* weights, const float* wavelengths, float* x, float* y, float* z, size_t elems);
	static void toXYZComposite(const float* weights, const float* wavelengths, float& x, float& y, float& z, size_t elems);

private:
	std::unique_ptr<struct _SpectralMapperInternal> mInternal;
};
} // namespace PR