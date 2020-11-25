#pragma once

#include "PR_Config.h"

namespace PR {
namespace Scattering {
// Every function with a _global suffix is only useable in global space where the view vector is incident,
// as opposed to the outgoing view vector in shading space!

/**
* @param gNdotV dot product between normal and incident view vector (global space)
*/
inline bool is_inside_global(float gNdotV)
{
	return gNdotV > PR_EPSILON;
}

/**
* @param gNdotV dot product between normal and incident view vector
* @param N Normal of the surface point.
*/
inline Vector3f faceforward_global(float gNdotV, const Vector3f& N)
{
	return is_inside_global(gNdotV) ? -N : N;
}

/////////////////////////////////////////////
/**
* @brief Returns simple correction factor between shading and geometry normal
*
* @todo Make use of this!
* @param Ng Surface geometry normal
* @param Ns Surface shading normal
* @param V Unit vector pointing FROM or TO the surface point in shading space.
* @return Factor between [0, 1] correcting the loss of energy by changing from geometry to shading
*/
inline float normal_correction_factor_global(const Vector3f& Ng, const Vector3f& Ns, const Vector3f& V)
{
	const float denominator = std::abs(Ng.dot(V));
	return denominator <= PR_EPSILON ? 1 : std::abs(Ns.dot(V) / denominator);
}

////////////////////////////////////////////////
/**
* @param V Outgoing vector
* @param N Normal to faceforward with.
*/
inline Vector3f faceforward(const Vector3f& V, const Vector3f& N)
{
	return V.dot(N) < 0 ? -V : V;
}

/**
* Optimized version in shading space
* @param V Outgoing vector
*/
inline Vector3f faceforward(const Vector3f& V)
{
	return V(2) < 0 ? -V : V;
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
* @brief Reflects the outgoing viewing vector through a user given normal.
* L = 2(N*V)N - V
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
* @brief Refracts the ray based on the eta parameter (eta = n1/n2) and previously calculated NdotT (Angle between a custom normal and refracted ray)
*
* @param eta Index ratio (n1/n2) between the two mediums.
* @param NdotT Angle between N and the result of this function
* @param NdotV Angle between N and V
* @param V Unit vector pointing FROM the surface point in shading space.
* @param N Unit vector pointing FROM the surface point in shading space corresponding to a custom normal.
* @return Unit vector pointing FROM the surface point outwards in shading space.
*/
inline Vector3f refract(float eta, float NdotT, float NdotV,
						const Vector3f& V, const Vector3f& N)
{
	return (-V * eta + (eta * NdotV - NdotT) * N).normalized();
}

/**
* @brief Refracts the ray based on the eta parameter (eta = n1/n2) and previously calculated NdotT (Angle between Normal and refracted ray)
*
* @param eta Index ratio (n1/n2) between the two mediums.
* @param NdotT Positive angle between N and the result of this function
* @param V Unit vector pointing FROM the surface point in shading space.
* @return Unit vector pointing FROM the surface point outwards in shading space.
*/
inline Vector3f refract(float eta, float NdotT,
						const Vector3f& V)
{
	// In case the view vector hits from below, flip the normal, which results into a flip in sign
	const float k = V(2) < 0 ? NdotT : -NdotT;
	return Vector3f(-V(0) * eta, -V(1) * eta, k).normalized();
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

inline Vector3f halfway_reflection(const Vector3f& V, const Vector3f& L)
{
	return (V + L).normalized();
}

inline Vector3f halfway_transmission(float inv_eta, const Vector3f& V, const Vector3f& L)
{
	return -(V + inv_eta * L).normalized();
}

inline Vector3f halfway_transmission(float n1, const Vector3f& V, float n2, const Vector3f& L)
{
	return halfway_transmission(n2 / n1, V, L);
}

/////////////////////////////////////////////

/**
* @brief Returns the refractive index based on the basic cauchy equation  
* @param lambda_nm Wavelength in nano meters
* @param Cs Coefficients
* @param n Amount of coefficients
* @return Refractive index
*/
template <typename T1, typename T2>
inline T1 cauchy(const T1& lambda_nm, const T2* Cs, size_t n)
{
	const T1 lambda_qm	= lambda_nm / 1000;
	const T1 lambda_qm2 = lambda_qm * lambda_qm;
	const T1 inv_l		= 1 / lambda_qm2;
	T1 denom			= inv_l;
	T1 value			= T1(Cs[0]);
	PR_OPT_LOOP
	for (size_t i = 1; i < n; ++i) {
		value += Cs[i] * denom;
		denom *= inv_l;
	}
	return value;
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
	PR_OPT_LOOP
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
} // namespace Scattering
} // namespace PR
