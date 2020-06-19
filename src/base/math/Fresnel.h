#pragma once

#include "Reflection.h"

namespace PR {
namespace Fresnel {
/* V is outgoing! -> NdotV is positive! */
inline float dielectric(float NdotV, float NdotT, float n1, float n2)
{
	/*if(NdotV*NdotT <= PR_EPSILON)
			return 1;*/

	const float para = diffProd(n1, NdotV, n2, NdotT) / sumProd(n1, NdotV, n2, NdotT);
	const float perp = diffProd(n1, NdotT, n2, NdotV) / sumProd(n1, NdotT, n2, NdotV);

	return sumProd(para, para, perp, perp) / 2.0f;
}

inline float dielectric(float NdotV, float n1, float n2)
{
	// Snells Law
	const float NdotT = Reflection::refraction_angle(NdotV, n1 / n2);

	if (NdotT < 0)
		return 1;

	return dielectric(NdotV, NdotT, n1, n2);
}

inline float conductor(float dot, float n, float k)
{
	if (dot <= PR_EPSILON)
		return 1;

	const float dot2 = dot * dot;
	const float f	 = sumProd(n, n, k, k);
	const float d1	 = f * dot2;
	const float d2	 = 2 * n * dot;

	const float para = (d1 - d2) / (d1 + d2);
	const float perp = (f - d2 + dot2) / (f + d2 + dot2);

	const float R = sumProd(para, para, perp, perp) / 2;
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
