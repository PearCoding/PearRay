#include "SpectrumDescriptor.h"

#include "SIMath.h"
#include "SIMathConstants.h"
#include "SIMathStd.h"

#include <type_traits>

namespace PR {
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

Spectrum SpectrumDescriptor::fromBlackbody(float temp) const
{
	SI::TemperatureU<long double, 0> T(temp);
	Spectrum spec(this);
	for (uint32 i = 0; i < samples(); ++i) {
		long double lambda = wavelength(i) * PR_NM_TO_M_F;
		spec.setValue(i, static_cast<float>(
			(long double)blackbody_eq(
				T,
				SI::LengthU<long double, 0>(lambda))));
	}

	return spec;
}
}
