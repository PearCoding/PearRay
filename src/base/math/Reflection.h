#pragma once

#include "PR_Config.h"

namespace PR {
namespace Reflection {
// Every function with a _global suffix is only useable in global space where the view vector is incident,
// as opposed to the outgoing view vector in shading space!

/**
* @param NdotV dot product between normal and incident view vector (global space)
*/
inline bool is_inside_global(float gNdotV)
{
	return gNdotV > PR_EPSILON;
}

/**
* @param NdotV dot product between normal and incident view vector
* @param N Normal of the surface point.
*/
inline Vector3f faceforward_global(float gNdotV, const Vector3f& N)
{
	return is_inside_global(gNdotV) ? -N : N;
}

////////////////////////////////////////////////

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
* @brief Reflects the outgoing viewing vector through the surface normal.
* L = 2(N*V)N - V
* As we are in shading space this simplifies to (-V0, -V1, V2)
* 
* @param V Unit vector pointing FROM the surface point in shading space.
* @return Unit vector pointing FROM the surface point outwards in shading space.
*/
inline Vector3f reflect(const Vector3f& V)
{
	return Vector3f(-V(0), -V(1), V(2));
}

/**
* @brief Reflects the outgoing viewing vector through the a normal.
* L = 2(N*V)N - V
* As we are in shading space this simplifies to (-V0, -V1, V2)
* 
* @param N Unit vector pointing FROM the surface point in shading space.
* @param V Unit vector pointing FROM the surface point in shading space.
* @return Unit vector pointing FROM the surface point outwards in shading space.
*/
inline Vector3f reflect(const Vector3f& V, const Vector3f& N)
{
	return 2 * N.dot(V) * N - V;
}

/**
* @brief Refracts the ray based on the eta parameter (eta = n1/n2) and previously calculated NdotT (Angle between Normal and refracted ray)
*
* @param eta Index ratio (n1/n2) between the two mediums.
* @param NdotT Angle between N and the result of this function
* @param V Unit vector pointing FROM the surface point in shading space.
* @return Unit vector pointing FROM the surface point outwards in shading space.
*/
inline Vector3f refract(float eta, float NdotT,
						const Vector3f& V)
{
	return Vector3f(-V(0) * eta, -V(1) * eta, -NdotT).normalized();
}

/**
* @brief Refracts the ray based on the eta parameter (eta = n1/n2) and stops when total reflection.
*
* @param eta Index ratio (n1/n2) between the two mediums.
* @param V Unit vector pointing FROM the surface point in shading space.
* @return Unit vector pointing FROM the surface point outwards in shading space.
*/
inline Vector3f refract(float eta, const Vector3f& V, bool& total)
{
	const float NdotT = refraction_angle(V(2), eta);

	total = NdotT < 0.0f;
	if (total) //TOTAL REFLECTION
		return Vector3f(0, 0, 0);
	else
		return refract(eta, NdotT, V);
}

/**
* @brief Refracts the ray based on the eta parameter (eta = n1/n2)
*
* @param eta Index ratio (n1/n2) between the two mediums.
* @param V Unit vector pointing FROM the surface point in shading space.
* @return Unit vector pointing FROM the surface point outwards in shading space.
*/
inline Vector3f refract(float eta, const Vector3f& V)
{
	const float NdotT = refraction_angle(V(2), eta);

	if (NdotT < 0.0f) //TOTAL REFLECTION
		return reflect(V);
	else
		return refract(eta, NdotT, V);
}

/////////////////////////////////////////////

inline Vector3f halfway(const Vector3f& V, const Vector3f& L)
{
	return (V + L).normalized();
}

/////////////////////////////////////////////

/**
* @brief Returns the refractive index based on the simple cauchy equation  
* @param lambda_nm Wavelength in nano meters
* @param A A base coefficient
* @param B B coefficient given in squared micro meters
* @return Refractive index
*/
template <typename T1, typename T2>
inline T1 cauchy(const T1& lambda_nm, const T2& A, const T2& B)
{
	const T1 lambda_qm	= lambda_nm / 1000;
	const T1 lambda_qm2 = lambda_qm * lambda_qm;
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
template <typename T1, typename T2>
inline T1 cauchy(const T1& lambda_nm, const T2& A, const T2& B, const T2& C)
{
	const T1 lambda_qm	= lambda_nm / 1000;
	const T1 lambda_qm2 = lambda_qm * lambda_qm;
	return A + B / lambda_qm2 + C / (lambda_qm2 * lambda_qm2);
}

/**
* @brief Returns the (squared) refractive index based on the sellmeier equation  
* @param lambda_nm Wavelength in nano meters
* @param Bs B coefficients N times
* @param Cs C coefficients given in squared micro meters N times
* @param n Amount of coefficients (pairs) given
* @return Squared refractive index
*/
template <typename T1, typename T2>
inline T1 sellmeier2(const T1& lambda_nm, const T2* Bs, const T2* Cs, size_t n)
{
	T1 lambda_qm  = lambda_nm / 1000;
	T1 lambda_qm2 = lambda_qm * lambda_qm;
	T1 value	  = T1(1);
	for (size_t i = 0; i < n; ++i)
		value += Bs[i] * lambda_qm2 / (lambda_qm2 - Cs[i]);
	return value;
}

/**
* @brief Returns the refractive index based on the sellmeier equation  
* @param lambda_nm Wavelength in nano meters
* @param Bs B coefficients N times
* @param Cs C coefficients given in squared micro meters N times
* @param n Amount of coefficients given
* @return Refractive index
*/
template <typename T1, typename T2>
inline T1 sellmeier(const T1& lambda_nm, const T2* Bs, const T2* Cs, size_t n)
{
	return std::sqrt(sellmeier2<T1, T2>(lambda_nm, Bs, Cs, n));
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
template <typename T1, typename T2>
inline T1 poly2(const T1& lambda_nm, const T2& A, const T2& B1, const T2& B2, const T2& C1, const T2& C2)
{
	T1 lambda_qm  = lambda_nm / 1000;
	T1 lambda_qm2 = lambda_qm * lambda_qm;
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
