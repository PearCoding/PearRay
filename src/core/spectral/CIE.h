#pragma once

#include "SpectralBlob.h"
#include "sampler/Distribution1D.h"

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
			/*if (weight[k] != 0.0f)
				++counter;*/
			const float f	= std::min<float>(PR_CIE_SAMPLE_COUNT - 2,
											  std::max(0.0f, (wavelength[k] - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_DELTA));
			const int index = f;
			const float t	= f - index;

			xyz[0] += weight[k] * (NM_TO_X[index] * (1 - t) + NM_TO_X[index + 1] * t);
			xyz[1] += weight[k] * (NM_TO_Y[index] * (1 - t) + NM_TO_Y[index + 1] * t);
			xyz[2] += weight[k] * (NM_TO_Z[index] * (1 - t) + NM_TO_Z[index + 1] * t);
		}

		//xyz *= PR_CIE_NORM;
	}

	static inline float sample_y(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_Y_INTEGRAL, CDF_Y, PR_CIE_SAMPLE_COUNT + 1);
		return v * (PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START) + PR_CIE_WAVELENGTH_START;
	}

	static inline float sample_xyz(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_SUM_INTEGRAL, CDF_SUM, PR_CIE_SAMPLE_COUNT + 1);
		return v * (PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START) + PR_CIE_WAVELENGTH_START;
	}

private:
	static const float NM_TO_X[];
	static const float NM_TO_Y[];
	static const float NM_TO_Z[];

	static const float CDF_SUM[];
	static const float CDF_SUM_INTEGRAL;
	static const float CDF_Y[];
	static const float CDF_Y_INTEGRAL;
};
} // namespace PR