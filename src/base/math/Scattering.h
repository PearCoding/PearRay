#pragma once

#include "PR_Config.h"

namespace PR {
namespace Scattering {
/////////////////////////////////////////////
/**
* @brief Returns simple correction factor between shading and geometry normal
*
* @todo Make use of this!
* @param Ng Surface geometry normal
* @param Ns Surface shading normal
* @param wIn Unit vector pointing FROM or TO the surface point in shading space.
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
* @param eta Index ratio (n_in/n_out) between the two mediums.
* @param cosI Angle between N and light
* @return cosT Angle between N and the (virtual) refracted ray. -1 if total reflection!
*/
inline float refraction_angle(float cosI, float eta)
{
	if (std::signbit(cosI)) // Negative hemisphere
		return refraction_angle(-cosI, 1 / eta);

	const float k = 1 - (eta * eta) * (1 - cosI * cosI);
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
* @brief Refracts the ray based on the eta parameter (eta = n_in/n_out) and stops when total reflection.
*
* @param eta Index ratio (n_in/n_out) between the two mediums.
* @param wIn Unit vector pointing FROM the surface point in shading space.
* @return wOut Unit vector pointing FROM the surface point outwards in shading space.
*/
inline Vector3f refract(float eta, const Vector3f& wIn)
{
	if (std::signbit(wIn(2))) // Negative hemisphere
		return -refract(1 / eta, -wIn);

	const float cosT = refraction_angle(wIn(2), eta);

	if (cosT < 0.0f) //TOTAL REFLECTION
		return reflect(wIn);
	else
		return Vector3f(-wIn(0) * eta, -wIn(1) * eta, -cosT).normalized();
}

/**
* @brief Refracts the ray based on the eta parameter (eta = n_in/n_out)
*
* @param eta Index ratio (n_in/n_out) between the two mediums.
* @param wIn Unit vector pointing FROM the surface point in shading space.
* @param N Unit vector pointing FROM the surface point in shading space. Should be the normal giving the orientation of the surface
* @param total True if total reflection occured
* @return Unit vector pointing FROM the surface point outwards in shading space.
*/
inline Vector3f refract(float eta, const Vector3f& wIn, const Vector3f& N, bool& total)
{
	const float cosI = wIn.dot(N);

	if (std::signbit(cosI)) // Negative hemisphere
		return -refract(1 / eta, -wIn, N, total);

	const float cosT = refraction_angle(cosI, eta);
	total			 = cosT < 0.0f;

	if (total) //TOTAL REFLECTION
		return reflect(wIn, N);
	else
		return (-wIn * eta + (eta * cosI - cosT) * N).normalized();
}

/**
* @brief Refracts the ray based on the eta parameter (eta = n_in/n_out)
*
* @param eta Index ratio (n_in/n_out) between the two mediums.
* @param wIn Unit vector pointing FROM the surface point in shading space.
* @param N Unit vector pointing FROM the surface point in shading space. Should be the normal giving the orientation of the surface
* @return Unit vector pointing FROM the surface point outwards in shading space.
*/
inline Vector3f refract(float eta, const Vector3f& wIn, const Vector3f& N)
{
	bool _ignore;
	return refract(eta, wIn, N, _ignore);
}

/////////////////////////////////////////////

/// Calculate halfway vector for reflection
inline Vector3f halfway_reflection(const Vector3f& wIn, const Vector3f& wOut)
{
	// No need to check if zero. Eigen3 will handle it
	return (wIn + wOut).normalized();
}

/// Calculate halfway vector for refraction
inline Vector3f halfway_refractive(float eta, const Vector3f& wIn, const Vector3f& wOut)
{
	// No need to check if zero. Eigen3 will handle it
	return -(eta * wIn + wOut).normalized();
}

/// Calculate halfway vector for refraction
inline Vector3f halfway_refractive(float n_in, const Vector3f& wIn, float n_out, const Vector3f& wOut)
{
	// No need to check if zero. Eigen3 will handle it
	return -(n_in * wIn + n_out * wOut).normalized();
}

/// Calculate halfway vector for reflection
inline static float reflective_jacobian(float cosO)
{
	const float denom = 4 * std::abs(cosO);
	return denom <= PR_EPSILON ? 0.0f : 1 / denom;
}

/// Calculate refraction jacobian
/// @param eta Index ratio (n_in/n_out) between the two mediums.
inline static float refractive_jacobian(float eta, float cosI, float cosO)
{
	const float denom  = eta * cosI + cosO; // Unsigned length of (unnormalized) refractive H
	const float denom2 = denom * denom;
	return denom2 <= PR_EPSILON ? 0.0f : std::abs(cosO) / denom2;
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
