#include "Spectrum.h"

#include <type_traits>
static_assert(std::is_standard_layout<PR::Spectrum>::value,
			  "Spectrum is not a standard layout type");

static_assert(sizeof(PR::Spectrum) == PR::Spectrum::SAMPLING_COUNT * sizeof(float),
			  "Spectrum is not a same size as internal data");

namespace PR {
#include "xyz.inl"

float Spectrum::approx(float wavelength, InterpolationType interpolation) const
{
	if (wavelength < WAVELENGTH_START || wavelength > WAVELENGTH_END)
		return -1;

	float st = wavelength - WAVELENGTH_START;
	uint32 c = (uint32)(st / WAVELENGTH_STEP);
	uint32 m = (uint32)fmodf(st, (float)WAVELENGTH_STEP);

	if (st < 0 || c >= SAMPLING_COUNT)
		return 0;

	if (interpolation == IT_Const) {
		if (m >= WAVELENGTH_STEP / 2 && c < SAMPLING_COUNT - 1) {
			return mValues[c + 1];
		} else {
			return mValues[c];
		}
	} else {
		if (c >= SAMPLING_COUNT - 1) {
			return mValues[SAMPLING_COUNT - 1];
		} else {
			float t = m / (float)WAVELENGTH_STEP;
			return mValues[c] * (1 - t) + mValues[c + 1] * t;
		}
	}
}

constexpr float CANDELA = 683.002f;
void Spectrum::weightPhotometric()
{
	for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		mValues[i] *= NM_TO_Y[i] * CANDELA;
}

float Spectrum::luminousFlux() const
{
	float flux = 0;
	for (uint32 i = 0; i < SAMPLING_COUNT; ++i)
		flux += mValues[i] * NM_TO_Y[i];

	return flux * ILL_SCALE * CANDELA;
}

// Has to be in double!
inline long double blackbody_eq(long double temp, long double lambda_nm)
{
	constexpr long double c  = 299792458l;
	constexpr long double h  = 6.626070040e-34l;
	constexpr long double kb = 1.38064852e-23l;

	constexpr long double c1 = 2 * h * c * c;
	constexpr long double c2 = h * c / kb;

	const long double lambda5 = lambda_nm * (lambda_nm * lambda_nm) * (lambda_nm * lambda_nm);
	return (c1 / lambda5) / (std::exp(c2 / (lambda_nm * temp)) - 1);
}

Spectrum Spectrum::fromBlackbody(float temp)
{
	Spectrum spec;
	for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i) {
		long double lambda = (WAVELENGTH_START + i * WAVELENGTH_STEP) * 1e-9l;
		spec.mValues[i]	= static_cast<float>(blackbody_eq(temp, lambda));
	}

	return spec;
}
}
