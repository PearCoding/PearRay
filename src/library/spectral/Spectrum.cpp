#include "Spectrum.h"

#include "SIMath.h"
#include "SIMathConstants.h"
#include "SIMathStd.h"

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
template <typename T = long double>
inline SI::SpectralIrradianceWavelengthU<T, 0>
blackbody_eq(const SI::TemperatureU<T, 0>& temp, const SI::LengthU<T, 0>& lambda_nm)
{
	using namespace SI;

	const auto c  = SI::constants::c<T>();
	const auto h  = SI::constants::h<T>();
	const auto kb = SI::constants::kb<T>();

	const auto c1 = 2 * h * c * c;
	const auto c2 = h * c / kb;

	const auto lambda5 = lambda_nm * (lambda_nm * lambda_nm) * (lambda_nm * lambda_nm);
	const auto f	   = c2 / (lambda_nm * temp);

	return (c1 / lambda5) / SI::expm1(f);
}

Spectrum Spectrum::fromBlackbody(float temp)
{
	SI::TemperatureU<long double, 0> T(temp);
	Spectrum spec;
	for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i) {
		long double lambda = (WAVELENGTH_START + i * WAVELENGTH_STEP) * 1e-9l;
		spec.mValues[i]	= static_cast<float>(
			(long double)blackbody_eq(
				T,
				SI::LengthU<long double, 0>(lambda)));
	}

	return spec;
}
}
