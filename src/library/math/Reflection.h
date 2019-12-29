#pragma once

#include "PR_Config.h"

namespace PR {
namespace Reflection {
/**
* @param NdotV dot product between normal and incident view vector
*/
inline bool is_inside(float NdotV)
{
	return NdotV > PR_EPSILON;
}

/**
* @param NdotV dot product between normal and incident view vector
* @param N Normal of the surface point.
*/
inline Vector3f faceforward(float NdotV, const Vector3f& N)
{
	return is_inside(NdotV) ? -N : N;
}

/**
* @brief Returns angle between normal and refracted ray based on the Snell's law.
*
* @param eta Index ratio (n1/n2) between the two mediums.
* @param NdotV Angle between N and V
* @return NdotT Angle between N and the (virtual) refracted ray. -1 if total reflection!
*/
inline float refraction_angle(float NdotV, float eta)
{
	const float k = 1 - (eta * eta) * (1 - NdotV * NdotV);
	if (k < 0)
		return -1;
	else
		return std::sqrt(k);
}

/**
* @brief Reflects the viewing vector through the surface normal.
* L = V - 2(N*V)N
*
* @param NdotV Angle between N and V
* @param N Normal of the surface point.
* @param V Unit vector pointing TO the surface point.
* @return Unit vector pointing FROM the surface point outwards.
*/
inline Vector3f reflect(float NdotV, const Vector3f& N, const Vector3f& V)
{
	/*if (is_inside(NdotV)) // Backfacing
		return (V + N * 2 * NdotV).normalized();
	else*/
		return (V - N * 2 * NdotV).normalized();
}

/**
* @brief Refracts the ray based on the eta parameter (eta = n1/n2) and previously calculated NdotT (Angle between Normal and refracted ray)
*
* @param eta Index ratio (n1/n2) between the two mediums.
* @param NdotV Angle between N and V
* @param NdotT Angle between N and the result of this function
* @param N Normal of the surface point.
* @param V Unit vector pointing TO the surface point.
* @return Unit vector pointing FROM the surface point outwards.
*/
inline Vector3f refract(float eta, float NdotV, float NdotT,
						const Vector3f& N, const Vector3f& V)
{
	const float t = -eta * NdotV - NdotT;
	return (V * eta + N * t).normalized();
}

/**
* @brief Refracts the ray based on the eta parameter (eta = n1/n2) and stops when total reflection.
*
* @param eta Index ratio (n1/n2) between the two mediums.
* @param NdotV Angle between N and V
* @param N Normal of the surface point.
* @param V Unit vector pointing TO the surface point.
* @return Unit vector pointing FROM the surface point outwards.
*/
inline Vector3f refract(float eta, float NdotV, const Vector3f& N, const Vector3f& V, bool& total)
{
	const float NdotT = refraction_angle(NdotV, eta);

	total = NdotT < 0.0f;
	if (total) //TOTAL REFLECTION
		return Vector3f(0, 0, 0);
	else
		return refract(eta, NdotV, NdotT, N, V);
}

/**
* @brief Refracts the ray based on the eta parameter (eta = n1/n2)
*
* @param eta Index ratio (n1/n2) between the two mediums.
* @param NdotV Angle between N and V
* @param N Normal of the surface point.
* @param V Unit vector pointing TO the surface point.
* @return Unit vector pointing FROM the surface point outwards.
*/
inline Vector3f refract(float eta, float NdotV, const Vector3f& N, const Vector3f& V)
{
	const float NdotT = refraction_angle(NdotV, eta);

	if (NdotT < 0.0f) //TOTAL REFLECTION
		return reflect(NdotV, N, V);
	else
		return refract(eta, NdotV, NdotT, N, V);
}

/**
* @brief Returns the halfway vector between V and L.
* @param V Unit vector pointing TO the surface point.
* @param L Unit vector pointing FROM the surface point outwards.
* @return Unit vector pointing FROM the surface point outwards. (Between L and V)
*/
inline Vector3f halfway(const Vector3f& V, const Vector3f& L)
{
	return (L - V).normalized();
}
} // namespace Reflection
} // namespace PR
