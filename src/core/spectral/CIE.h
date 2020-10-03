#pragma once

#include "EquidistantSpectrum.h"
#include "SpectralBlob.h"
#include "math/Distribution1D.h"

namespace PR {
using CIETriplet = Eigen::Array<float, 3, 1>;

// The spectrum the CIE data is available
constexpr int PR_CIE_SAMPLE_COUNT		= 95;
constexpr float PR_CIE_WAVELENGTH_START = 360;
constexpr float PR_CIE_WAVELENGTH_END	= 830;
constexpr float PR_CIE_WAVELENGTH_RANGE = PR_CIE_WAVELENGTH_END - PR_CIE_WAVELENGTH_START;
constexpr float PR_CIE_WAVELENGTH_DELTA = PR_CIE_WAVELENGTH_RANGE / (PR_CIE_SAMPLE_COUNT - 1);

// The default visible spectrum constants based on
// Starr, Cecie (2005). Biology: Concepts and Applications. Thomson Brooks/Cole. p. 94. ISBN 978-0-534-46226-0.
// but with a slightly larger domain [360,760] instead of [380,740]
constexpr float PR_VISIBLE_WAVELENGTH_START = 360;
constexpr float PR_VISIBLE_WAVELENGTH_END	= 760;
constexpr float PR_VISIBLE_WAVELENGTH_DELTA = PR_CIE_WAVELENGTH_DELTA;
constexpr float PR_VISIBLE_WAVELENGTH_RANGE = PR_VISIBLE_WAVELENGTH_END - PR_VISIBLE_WAVELENGTH_START;
constexpr int PR_VISIBLE_SAMPLE_COUNT		= PR_VISIBLE_WAVELENGTH_RANGE / PR_VISIBLE_WAVELENGTH_DELTA + 1;

static_assert(PR_CIE_WAVELENGTH_START <= PR_VISIBLE_WAVELENGTH_START
				  && PR_VISIBLE_WAVELENGTH_END <= PR_CIE_WAVELENGTH_END,
			  "The default visible spectrum has to be a subset of the CIE spectrum");

class PR_LIB_CORE CIE {
public:
	static inline CIETriplet eval(const SpectralBlob& weight, const SpectralBlob& wavelength)
	{
		CIETriplet xyz = CIETriplet::Zero();

		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t k = 0; k < PR_SPECTRAL_BLOB_SIZE; ++k)
			xyz += weight[k] * eval(wavelength[k]);

		return xyz;
	}

	static inline CIETriplet eval(float wavelength)
	{
		static const EquidistantSpectrumView CIE_X(NM_TO_X, PR_CIE_SAMPLE_COUNT, PR_CIE_WAVELENGTH_START, PR_CIE_WAVELENGTH_END);
		static const EquidistantSpectrumView CIE_Y(NM_TO_Y, PR_CIE_SAMPLE_COUNT, PR_CIE_WAVELENGTH_START, PR_CIE_WAVELENGTH_END);
		static const EquidistantSpectrumView CIE_Z(NM_TO_Z, PR_CIE_SAMPLE_COUNT, PR_CIE_WAVELENGTH_START, PR_CIE_WAVELENGTH_END);

		return CIETriplet{ CIE_X.lookup(wavelength), CIE_Y.lookup(wavelength), CIE_Z.lookup(wavelength) };
	}

	//////////////// CIE
	////////////////
	static inline float eval_y(float wavelength)
	{
		static const EquidistantSpectrumView CIE_Y(NM_TO_Y, PR_CIE_SAMPLE_COUNT, PR_CIE_WAVELENGTH_START, PR_CIE_WAVELENGTH_END);
		return CIE_Y.lookup(wavelength);
	}

	static inline float sample_y(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_Y);
		return v * PR_CIE_WAVELENGTH_RANGE + PR_CIE_WAVELENGTH_START;
	}

	static inline float pdf_y(float wvl)
	{
		return Distribution1D::continuousPdf((wvl - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_RANGE, CDF_Y);
	}

	////////////////
	static inline float sample_trunc_y(float u, float& pdf, float start, float end)
	{
		const float norm_start = (start - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_RANGE;
		const float norm_end   = (end - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_RANGE;

		const float v = sample_trunc(u, pdf, norm_start, norm_end, CDF_Y);
		return v * (end - start) + start;
	}

	static inline float pdf_trunc_y(float wvl, float start, float end)
	{
		const float norm_start = (start - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_RANGE;
		const float norm_end   = (end - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_RANGE;
		return pdf_trunc(pdf_y(wvl), norm_start, norm_end, CDF_Y);
	}

	////////////////
	static inline float sample_xyz(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_XYZ);
		return v * PR_CIE_WAVELENGTH_RANGE + PR_CIE_WAVELENGTH_START;
	}

	static inline float pdf_xyz(float wvl)
	{
		return Distribution1D::continuousPdf((wvl - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_RANGE, CDF_XYZ);
	}

	////////////////
	static inline float sample_trunc_xyz(float u, float& pdf, float start, float end)
	{
		const float norm_start = (start - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_RANGE;
		const float norm_end   = (end - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_RANGE;

		const float v = sample_trunc(u, pdf, norm_start, norm_end, CDF_XYZ);
		return v * (end - start) + start;
	}

	static inline float pdf_trunc_xyz(float wvl, float start, float end)
	{
		const float norm_start = (start - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_RANGE;
		const float norm_end   = (end - PR_CIE_WAVELENGTH_START) / PR_CIE_WAVELENGTH_RANGE;
		return pdf_trunc(pdf_xyz(wvl), norm_start, norm_end, CDF_XYZ);
	}

	//////////////// VISIBLE
	////////////////
	static inline float sample_vis_y(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_VIS_Y);
		return v * PR_VISIBLE_WAVELENGTH_RANGE + PR_VISIBLE_WAVELENGTH_START;
	}

	static inline float pdf_vis_y(float wvl)
	{
		return Distribution1D::continuousPdf((wvl - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_RANGE, CDF_VIS_Y);
	}

	////////////////
	static inline float sample_trunc_vis_y(float u, float& pdf, float start, float end)
	{
		const float norm_start = (start - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_RANGE;
		const float norm_end   = (end - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_RANGE;

		const float v = sample_trunc(u, pdf, norm_start, norm_end, CDF_VIS_Y);
		return v * (end - start) + start;
	}

	static inline float pdf_trunc_vis_y(float wvl, float start, float end)
	{
		const float norm_start = (start - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_RANGE;
		const float norm_end   = (end - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_RANGE;
		return pdf_trunc(pdf_vis_y(wvl), norm_start, norm_end, CDF_VIS_Y);
	}

	////////////////
	static inline float sample_vis_xyz(float u, float& pdf)
	{
		const float v = Distribution1D::sampleContinuous(u, pdf, CDF_VIS_XYZ);
		return v * PR_VISIBLE_WAVELENGTH_RANGE + PR_VISIBLE_WAVELENGTH_START;
	}

	static inline float pdf_vis_xyz(float wvl)
	{
		return Distribution1D::continuousPdf((wvl - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_RANGE, CDF_VIS_XYZ);
	}

	////////////////
	static inline float sample_trunc_vis_xyz(float u, float& pdf, float start, float end)
	{
		const float norm_start = (start - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_RANGE;
		const float norm_end   = (end - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_RANGE;

		const float v = sample_trunc(u, pdf, norm_start, norm_end, CDF_VIS_XYZ);
		return v * (end - start) + start;
	}

	static inline float pdf_trunc_vis_xyz(float wvl, float start, float end)
	{
		const float norm_start = (start - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_RANGE;
		const float norm_end   = (end - PR_VISIBLE_WAVELENGTH_START) / PR_VISIBLE_WAVELENGTH_RANGE;
		return pdf_trunc(pdf_vis_xyz(wvl), norm_start, norm_end, CDF_VIS_XYZ);
	}

private:
	template <size_t N>
	static inline float sample_trunc(float u, float& pdf, float norm_start, float norm_end, const StaticCDF<N>& cdf)
	{
		const float cdf_start = Distribution1D::evalContinuous(norm_start, cdf);
		const float cdf_end	  = Distribution1D::evalContinuous(norm_end, cdf);

		const float v = Distribution1D::sampleContinuous(cdf_start + u * (cdf_end - cdf_start), pdf, cdf);

		pdf /= (cdf_end - cdf_start);
		return v;
	}

	template <size_t N>
	static inline float pdf_trunc(float pdf, float norm_start, float norm_end, const StaticCDF<N>& cdf)
	{
		const float cdf_start = Distribution1D::evalContinuous(norm_start, cdf);
		const float cdf_end	  = Distribution1D::evalContinuous(norm_end, cdf);
		return pdf / (cdf_end - cdf_start);
	}

	static const float NM_TO_X[];
	static const float NM_TO_Y[];
	static const float NM_TO_Z[];

	static const StaticCDF<PR_CIE_SAMPLE_COUNT> CDF_XYZ;
	static const StaticCDF<PR_CIE_SAMPLE_COUNT> CDF_Y;

	static const StaticCDF<PR_VISIBLE_SAMPLE_COUNT> CDF_VIS_XYZ;
	static const StaticCDF<PR_VISIBLE_SAMPLE_COUNT> CDF_VIS_Y;
};
} // namespace PR