#pragma once

#include "EquidistantSpectrum.h"
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
			xyz[i] = eval(weights[i], wavelengths[i]);
	}

	static inline CIETriplet eval(const SpectralBlob& weight, const SpectralBlob& wavelength)
	{
		static const EquidistantSpectrumView CIE_X(NM_TO_X, PR_CIE_SAMPLE_COUNT, PR_CIE_WAVELENGTH_START, PR_CIE_WAVELENGTH_END);
		static const EquidistantSpectrumView CIE_Y(NM_TO_Y, PR_CIE_SAMPLE_COUNT, PR_CIE_WAVELENGTH_START, PR_CIE_WAVELENGTH_END);
		static const EquidistantSpectrumView CIE_Z(NM_TO_Z, PR_CIE_SAMPLE_COUNT, PR_CIE_WAVELENGTH_START, PR_CIE_WAVELENGTH_END);
		CIETriplet xyz = CIETriplet::Zero();

		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t k = 0; k < PR_SPECTRAL_BLOB_SIZE; ++k)
			xyz[0] += weight[k] * CIE_X.lookup(wavelength[k]);

		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t k = 0; k < PR_SPECTRAL_BLOB_SIZE; ++k)
			xyz[1] += weight[k] * CIE_Y.lookup(wavelength[k]);

		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t k = 0; k < PR_SPECTRAL_BLOB_SIZE; ++k)
			xyz[2] += weight[k] * CIE_Z.lookup(wavelength[k]);

		return xyz;
	}

	static inline float eval_y(float wavelength)
	{
		static const EquidistantSpectrumView CIE_Y(NM_TO_Y, PR_CIE_SAMPLE_COUNT, PR_CIE_WAVELENGTH_START, PR_CIE_WAVELENGTH_END);
		return CIE_Y.lookup(wavelength);
	}

	static inline float pdf_y(float u)
	{
		return Distribution1D::continuousPdf(u, CDF_Y, PR_CIE_SAMPLE_COUNT + 1);
	}

	static inline float sample_y(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_Y, PR_CIE_SAMPLE_COUNT + 1);
		return v * (PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START) + PR_CIE_WAVELENGTH_START;
	}

	static inline float sample_xyz(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_SUM, PR_CIE_SAMPLE_COUNT + 1);
		return v * (PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START) + PR_CIE_WAVELENGTH_START;
	}

private:
	static const float NM_TO_X[];
	static const float NM_TO_Y[];
	static const float NM_TO_Z[];

	static const float CDF_SUM[];
	static const float CDF_Y[];
};
} // namespace PR