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

/**
* @brief Returns the refractive index based on the simple cauchy equation  
* @param lambda_nm Wavelength in nano meters
* @param A A base coefficient
* @param B B coefficient given in squared micro meters
* @return Refractive index
*/
inline float cauchy(float lambda_nm, float A, float B)
{
	const float lambda_qm  = lambda_nm / 1000;
	const float lambda_qm2 = lambda_qm * lambda_qm;
	return A + B / lambda_qm2;
}

/**
* @brief Returns the refractive index based on the basic cauchy equation  
* @param lambda_nm Wavelength in nano meters
* @param A A base coefficient
* @param B B coefficient given in squared micro meters
* @param C C coefficient given in quadrupled micro meters
* @return Refractive index
*/
inline float cauchy(float lambda_nm, float A, float B, float C)
{
	const float lambda_qm  = lambda_nm / 1000;
	const float lambda_qm2 = lambda_qm * lambda_qm;
	return A + B / lambda_qm2 + C / (lambda_qm2 * lambda_qm2);
}

/**
* @brief Returns the (squared) refractive index based on the sellmeier equation  
* @param lambda_nm Wavelength in nano meters
* @param B1 B1 coefficient
* @param B2 B2 coefficient
* @param B3 B3 coefficient
* @param C1 C1 coefficient given in squared micro meters
* @param C2 C2 coefficient given in squared micro meters
* @param C3 C3 coefficient given in squared micro meters
* @return Squared refractive index
*/
inline float sellmeier2(float lambda_nm, float B1, float B2, float B3, float C1, float C2, float C3)
{
	float lambda_qm	 = lambda_nm / 1000;
	float lambda_qm2 = lambda_qm * lambda_qm;
	return 1
		   + B1 * lambda_qm2 / (lambda_qm2 - C1)
		   + B2 * lambda_qm2 / (lambda_qm2 - C2)
		   + B3 * lambda_qm2 / (lambda_qm2 - C3);
}

/**
* @brief Returns the refractive index based on the sellmeier equation  
* @param lambda_nm Wavelength in nano meters
* @param B1 B1 coefficient
* @param B2 B2 coefficient
* @param B3 B3 coefficient
* @param C1 C1 coefficient given in squared micro meters
* @param C2 C2 coefficient given in squared micro meters
* @param C3 C3 coefficient given in squared micro meters
* @return Refractive index
*/
inline float sellmeier(float lambda_nm, float B1, float B2, float B3, float C1, float C2, float C3)
{
	return std::sqrt(sellmeier2(lambda_nm, B1, B2, B3, C1, C2, C3));
}

/**
* @brief Returns the (squared) refractive index based on the polynom equation n^2=A+B1*l^2+B2*l^4+C1/l^2+C2/l^4
* @param lambda_nm Wavelength in nano meters
* @param A A base coefficient
* @param B1 B1 coefficient given in inverse squared micro meters
* @param B2 B2 coefficient given in inverse quadrupled micro meters
* @param C1 C1 coefficient given in squared micro meters
* @param C2 C2 coefficient given in quadrupled micro meters
* @return Squared refractive index
*/
inline float poly2(float lambda_nm, float A, float B1, float B2, float C1, float C2)
{
	float lambda_qm	 = lambda_nm / 1000;
	float lambda_qm2 = lambda_qm * lambda_qm;
	return A + B1 * lambda_qm2 + B2 * lambda_qm2 * lambda_qm2 + C1 / lambda_qm2 + C2 / (lambda_qm2 * lambda_qm2);
}

/**
* @brief Returns the refractive index based on the polynom equation n^2=A+B1*l^2+B2*l^4+C1/l^2+C2/l^4
* @param lambda_nm Wavelength in nano meters
* @param A A base coefficient
* @param B1 B1 coefficient given in inverse squared micro meters
* @param B2 B2 coefficient given in inverse quadrupled micro meters
* @param C1 C1 coefficient given in squared micro meters
* @param C2 C2 coefficient given in quadrupled micro meters
* @return Refractive index
*/
inline float poly(float lambda_nm, float A, float B1, float B2, float C1, float C2)
{
	return std::sqrt(poly2(lambda_nm, A, B1, B2, C1, C2));
}
} // namespace Reflection
} // namespace PR
