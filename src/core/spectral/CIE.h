#pragma once

#include "EquidistantSpectrum.h"
#include "SpectralBlob.h"
#include "sampler/Distribution1D.h"

namespace PR {
using CIETriplet = Eigen::Array<float, 3, 1>;

// The spectrum the CIE data is available
constexpr int PR_CIE_SAMPLE_COUNT		= 95;
constexpr float PR_CIE_WAVELENGTH_START = 360;
constexpr float PR_CIE_WAVELENGTH_END	= 830;
constexpr float PR_CIE_WAVELENGTH_DELTA = (PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START) / (PR_CIE_SAMPLE_COUNT - 1);

// The default visible spectrum constants based on
// Starr, Cecie (2005). Biology: Concepts and Applications. Thomson Brooks/Cole. p. 94. ISBN 978-0-534-46226-0.
// but with a slightly larger domain [360,760] instead of [380,740]
constexpr float PR_VISIBLE_WAVELENGTH_START = 360;
constexpr float PR_VISIBLE_WAVELENGTH_END	= 760;
constexpr float PR_VISIBLE_WAVELENGTH_DELTA = PR_CIE_WAVELENGTH_DELTA;
constexpr int PR_VISIBLE_SAMPLE_COUNT		= (PR_VISIBLE_WAVELENGTH_END - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_DELTA + 1;

static_assert(PR_CIE_WAVELENGTH_START <= PR_VISIBLE_WAVELENGTH_START
				  && PR_VISIBLE_WAVELENGTH_END <= PR_CIE_WAVELENGTH_END,
			  "The default visible spectrum has to be a subset of the CIE spectrum");

class PR_LIB_CORE CIE {
public:
	static inline void eval(const SpectralBlob* PR_RESTRICT const weights, const SpectralBlob* PR_RESTRICT const wavelengths,
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
		return Distribution1D::continuousPdf(u, CDF_Y);
	}

	static inline float sample_y(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_Y);
		return v * (PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START) + PR_CIE_WAVELENGTH_START;
	}

	static inline float sample_xyz(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_XYZ);
		return v * (PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START) + PR_CIE_WAVELENGTH_START;
	}

	static inline float sample_vis_y(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_VIS_Y);
		return v * (PR_VISIBLE_WAVELENGTH_END - PR_VISIBLE_WAVELENGTH_START) + PR_VISIBLE_WAVELENGTH_START;
	}

	static inline float sample_vis_xyz(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_VIS_XYZ);
		return v * (PR_VISIBLE_WAVELENGTH_END - PR_VISIBLE_WAVELENGTH_START) + PR_VISIBLE_WAVELENGTH_START;
	}

private:
	static const float NM_TO_X[];
	static const float NM_TO_Y[];
	static const float NM_TO_Z[];

	static const StaticCDF<PR_CIE_SAMPLE_COUNT> CDF_XYZ;
	static const StaticCDF<PR_CIE_SAMPLE_COUNT> CDF_Y;

	static const StaticCDF<PR_VISIBLE_SAMPLE_COUNT> CDF_VIS_XYZ;
	static const StaticCDF<PR_VISIBLE_SAMPLE_COUNT> CDF_VIS_Y;
};
} // namespace PR