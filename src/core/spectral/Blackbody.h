#pragma once

#include "PR_Config.h"
#include "math/SIMath.h"

namespace PR {

/// Calculates the blackbody curve value for one particular wavelength with given temperature
inline float blackbody(float lambda_nm, float temp_kelvin)
{
	long double lambda = SI::LengthU<long double, 0>(lambda_nm * PR_NM_TO_M_F);

	const auto c  = SI::constants::c<T>();
	const auto h  = SI::constants::h<T>();
	const auto kb = SI::constants::kb<T>();

	const auto c1 = 2 * h * c * c;
	const auto c2 = h * c / kb;

	const auto lambda5 = lambda * (lambda * lambda) * (lambda * lambda);
	const auto f	   = c2 / (lambda * temp);

	return static_cast<float>((c1 / lambda5) / SI::expm1(f));
}

} // namespace PR
