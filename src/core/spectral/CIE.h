#pragma once

#include "SpectralBlob.h"

namespace PR {
using CIETriplet						= Eigen::Array<float, 3, 1>;
constexpr int PR_CIE_SAMPLE_COUNT		= 95;
constexpr float PR_CIE_WAVELENGTH_START = 360;
constexpr float PR_CIE_WAVELENGTH_END	= 830;
constexpr float PR_CIE_WAVELENGTH_DELTA = (PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START) / (PR_CIE_SAMPLE_COUNT - 1);
constexpr float PR_CIE_Y_SUM			= 21.3714078505f;
constexpr float PR_CIE_NORM				= 1.0f / PR_CIE_Y_SUM;

class PR_LIB_CORE CIE {
public:
	static inline void eval(const SpectralBlob* const weights, const SpectralBlob* const wavelengths,
							CIETriplet* xyz,
							size_t entry_count)
	{
		PR_OPT_LOOP
		for (size_t i = 0; i < entry_count; ++i)
			eval(weights[i], wavelengths[i], xyz[i]);
	}

	static inline void eval(const SpectralBlob& weight, const SpectralBlob& wavelength,
							CIETriplet& xyz)
	{
		xyz = CIETriplet::Zero();

		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t k = 0; k < PR_SPECTRAL_BLOB_SIZE; ++k) {
			const float f = (std::max(PR_CIE_WAVELENGTH_START, wavelength[k]) - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_DELTA;

			const int index = std::max(0, std::min<int>(PR_CIE_SAMPLE_COUNT - 2, f));
			const float t	= f - index;

			xyz[0] += weight[k] * (NM_TO_X[index] * (1 - t) + NM_TO_X[index + 1] * t);
			xyz[1] += weight[k] * (NM_TO_Y[index] * (1 - t) + NM_TO_Y[index + 1] * t);
			xyz[2] += weight[k] * (NM_TO_Z[index] * (1 - t) + NM_TO_Z[index + 1] * t);
		}

		xyz *= PR_CIE_NORM;
	}

private:
	static const float NM_TO_X[];
	static const float NM_TO_Y[];
	static const float NM_TO_Z[];
};
} // namespace PR