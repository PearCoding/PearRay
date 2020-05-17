#pragma once

#include "SpectralBlob.h"

namespace PR {
using CIETriplet						= Eigen::Array<float, 3, 1>;
constexpr int PR_CIE_SAMPLE_COUNT		= 81;
constexpr float PR_CIE_WAVELENGTH_START = 360;
constexpr float PR_CIE_WAVELENGTH_END	= 830;
constexpr float PR_CIE_WAVELENGTH_DELTA = 5;
constexpr float PR_CIE_Y_SUM			= 2.137133e+01;

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
			float f = (std::max(PR_CIE_WAVELENGTH_START, wavelength[k]) - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_DELTA;

			float ft;
			int index = std::max(0, std::min<int>(PR_CIE_SAMPLE_COUNT - 1, std::modf(f, &ft)));

			if (index == PR_CIE_SAMPLE_COUNT - 1) {
				xyz[0] += weight[k] * NM_TO_X[index];
				xyz[1] += weight[k] * NM_TO_Y[index];
				xyz[2] += weight[k] * NM_TO_Z[index];
			} else {
				xyz[0] += weight[k] * (NM_TO_X[index] * ft + NM_TO_X[index + 1] * (1 - ft));
				xyz[1] += weight[k] * (NM_TO_Y[index] * ft + NM_TO_Y[index + 1] * (1 - ft));
				xyz[2] += weight[k] * (NM_TO_Z[index] * ft + NM_TO_Z[index + 1] * (1 - ft));
			}
		}
	}

private:
	static const float NM_TO_X[];
	static const float NM_TO_Y[];
	static const float NM_TO_Z[];
};
} // namespace PR