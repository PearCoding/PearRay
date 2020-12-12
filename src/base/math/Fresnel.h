#pragma once

#include "Scattering.h"

namespace PR {
namespace Fresnel {

/* V is outgoing! -> cosI is positive! */
inline float dielectric(float cosI, float cosO, float n_in, float n_out)
{
	PR_ASSERT(cosI >= 0, "cosI must be positive!");
	
	const float perp = diffProd(n_in, cosI, n_out, cosO) / sumProd(n_in, cosI, n_out, cosO);
	const float para = diffProd(n_out, cosI, n_in, cosO) / sumProd(n_out, cosI, n_in, cosO);

	return std::min(std::max(sumProd(para, para, perp, perp) / 2.0f, 0.0f), 1.0f);
}

inline float dielectric(float cosI, float n_in, float n_out)
{
	if (std::signbit(cosI)) // Negative hemisphere
		return dielectric(-cosI, n_out, n_in);

	// Snells Law
	const float cosT = Scattering::refraction_angle(cosI, n_in / n_out);

	if (cosT < 0)
		return 1;

	return dielectric(cosI, cosT, n_in, n_out);
}

inline float conductor(float cosI, float n_in, float n_out, float k)
{
	if(cosI < 0)
		cosI = -cosI;
		
	const float eta	  = n_out / n_in;
	const float kappa = k / n_in;

	const float cosI2  = cosI * cosI;
	const float sinI2  = 1 - cosI2;
	const float eta2   = eta * eta;
	const float kappa2 = kappa * kappa;

	const float t0	  = eta2 - kappa2 - sinI2;
	const float ap	  = std::sqrt(sumProd(t0, t0, 4 * eta2, kappa2));
	const float t1	  = ap + cosI2;
	const float a	  = std::sqrt((ap + t0) / 2);
	const float t2	  = 2 * cosI * a;
	const float perp2 = (t1 - t2) / (t1 + t2);

	const float t3	  = sumProd(cosI2, ap, sinI2, sinI2);
	const float t4	  = t2 * sinI2;
	const float para2 = perp2 * (t3 - t4) / (t3 + t4);

	const float R = (para2 + perp2) / 2;
	return std::min(std::max(R, 0.0f), 1.0f);
}

inline float schlick_term(float dot)
{
	const float t = 1 - dot;

	return (t * t) * (t * t) * t;
}

inline float schlick(float dot, float f0)
{
	return f0 + (1 - f0) * schlick_term(dot);
}

inline float schlick(float dot, float n1, float n2)
{
	const float c = (n1 - n2) / (n1 + n2);
	return schlick(dot, c * c);
}
} // namespace Fresnel
} // namespace PR
