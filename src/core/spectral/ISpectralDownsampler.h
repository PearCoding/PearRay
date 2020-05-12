#pragma once

#include "PR_Config.h"

namespace PR {
/// Downsamples continues (Hero) based wavelength dependent values to an arbitary (fixed sized) buffer
class PR_LIB_CORE ISpectralDownsampler {
public:
	/// Size of buffer required as an input to compute
	virtual size_t requiredOutputSize() const = 0;
	/// Downsample given collection of wavelengths
	virtual void compute(const float* weights, const float* wavelengths, float* output, size_t elems) = 0;
};
} // namespace PR