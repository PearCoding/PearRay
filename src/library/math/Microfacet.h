#pragma once

#include "math/Spherical.h"

namespace PR {
namespace Microfacet {
/* All functions are in the sampling space, with L and V!! are pointing out the surface.
 * This also means that NdotL, NdotV and NdotH can not be negative!
 */

////////////////////////////////
// Geometric Terms

inline float g_implicit(float NdotV, float NdotL)
{
	return NdotV * NdotL;
}

inline float g_neumann(float NdotV, float NdotL)
{
	return NdotV * NdotL / std::max(NdotV, NdotL);
}

inline float g_kelemen(float NdotV, float NdotL, float VdotH)
{
	return NdotV * NdotL / (VdotH * VdotH);
}

inline float g_cook_torrance(float NdotV, float NdotL, float NdotH, float VdotH)
{
	return std::min(1.0f, 2 * NdotH * std::min(NdotV, NdotL) / VdotH);
}

inline float g_1_schlick(float NdotK, float roughness)
{
	const float k = roughness * std::sqrt(PR_1_PI * 2.0f);
	return NdotK / (NdotK * (1 - k) + k);
}

inline float g_1_walter(float NdotK, float roughness)
{
	const float a = roughness * std::sqrt(1 - NdotK * NdotK) / NdotK;
	return a >= 1.6f
			   ? 1.0f
			   : (3.535f * a + 2.181f * a * a) / (1 + 2.276f * a + 2.577f * a * a);
}

// 1/(4*NdotV*NdotL) already multiplied out
inline float g_1_smith(float NdotK, float roughness)
{
	const float a = roughness * roughness;
	const float b = NdotK * NdotK;
	return 1.0f / (NdotK + std::sqrt(a + b - a * b));
}

// 1/(4*NdotV*NdotL) already multiplied out
inline float g_1_smith(float NdotK, float KdotX, float KdotY, float roughnessX, float roughnessY)
{
	const float kx = KdotX * roughnessX;
	const float ky = KdotY * roughnessY;
	const float b  = NdotK * NdotK;

	const float denom = NdotK + std::sqrt(kx * kx + ky * ky + b);
	if (denom <= PR_EPSILON)
		return 0.0f;
	else
		return 1.0f / denom;
}

// Not optimized
inline float g_1_smith_nopt(float NdotK, float KdotX, float KdotY, float roughnessX, float roughnessY)
{
	return 2 * NdotK * g_1_smith(NdotK, KdotX, KdotY, roughnessX, roughnessY);
}

/////////////////////////////////
// Distribution Terms

/* Normalized Beckmann distribution */
inline float ndf_beckmann(float NdotH, float roughness)
{
	const float k = NdotH * NdotH;
	const float m = roughness * roughness;
	return PR_1_PI * std::exp((k - 1.0f) / (k * m)) / (k * k * m);
}

inline float ndf_blinn(float NdotH, float roughness)
{
	const float alpha2 = 1.0f / (roughness * roughness);
	return std::pow(NdotH, 2.0f * alpha2 - 2.0f) * alpha2 * PR_1_PI;
}

inline float ndf_ggx(float NdotH, float roughness)
{
	const float alpha2 = roughness * roughness;
	const float dot2   = NdotH * NdotH;
	const float inv	= dot2 * (alpha2 - 1) + 1;
	const float inv_t2 = inv * inv;

	if (inv_t2 <= PR_EPSILON)
		return 0.0f;
	else
		return alpha2 * PR_1_PI / inv_t2;
}

inline float ndf_ggx(float NdotH, float HdotX, float HdotY, float roughnessX, float roughnessY)
{
	const float t = HdotX * HdotX / (roughnessX * roughnessX)
					+ HdotY * HdotY / (roughnessY * roughnessY)
					+ NdotH * NdotH;
	const float denom = roughnessX * roughnessY * t * t;
	if (denom <= PR_EPSILON)
		return 0.0f;
	else
		return PR_1_PI / denom;
}

////////////////////////////////
// Distribution Terms Sampling
inline Vector3f sample_ndf_beckmann(float u0, float u1, float roughness, float& pdf)
{
	float cosTheta, sinTheta; // V samples
	float cosPhi, sinPhi;	 // U samples
	float t  = 1 / (1 - roughness * roughness * std::log(1 - u1));
	cosTheta = std::sqrt(t);
	sinTheta = std::sqrt(1 - cosTheta * cosTheta);
	sinPhi   = std::sin(2 * PR_PI * u0);
	cosPhi   = std::cos(2 * PR_PI * u0);

	if (roughness <= PR_EPSILON)
		pdf = 1;
	else
		pdf = PR_1_PI / (roughness * roughness * cosTheta * cosTheta * cosTheta) * (1 - u1);

	return Spherical::cartesian(sinTheta, cosTheta, sinPhi, cosPhi);
}

inline float pdf_beckmann(float NdotH, float roughness)
{
	const float m = roughness * roughness;
	return PR_1_PI / (NdotH * NdotH * NdotH * m);
}

inline Vector3f sample_ndf_blinn(float u0, float u1, float roughness, float& pdf)
{
	float cosTheta, sinTheta; // V samples
	float cosPhi, sinPhi;	 // U samples
	if (roughness <= PR_EPSILON)
		cosTheta = 1;
	else
		cosTheta = std::pow(1 - u1, 1 / (2 * roughness * roughness));

	sinTheta = std::sqrt(1 - cosTheta * cosTheta);
	sinPhi   = std::sin(2 * PR_PI * u0);
	cosPhi   = std::cos(2 * PR_PI * u0);

	if (roughness <= PR_EPSILON)
		pdf = 1;
	else
		pdf = 4 * PR_PI * roughness * roughness * (1 - u1) / cosTheta;

	return Spherical::cartesian(sinTheta, cosTheta, sinPhi, cosPhi);
}

inline float pdf_blinn(float NdotH, float roughness)
{
	return 4 * PR_PI * roughness * roughness / NdotH;
}

inline Vector3f sample_ndf_ggx(float u0, float u1, float roughness, float& pdf)
{
	float cosTheta, sinTheta; // V samples
	float cosPhi, sinPhi;	 // U samples

	float alpha2 = roughness * roughness;
	float t2	 = alpha2 * u1 / (1 - u1);
	cosTheta	 = std::max(0.001f, 1.0f / std::sqrt(1 + t2));
	sinTheta	 = std::sqrt(1 - cosTheta * cosTheta);
	sinPhi		 = std::sin(2 * PR_PI * u0);
	cosPhi		 = std::cos(2 * PR_PI * u0);

	float s = 1 + t2 / alpha2;
	pdf		= PR_1_PI / (alpha2 * cosTheta * cosTheta * cosTheta * s * s);
	return Spherical::cartesian(sinTheta, cosTheta, sinPhi, cosPhi);
}

inline float pdf_ggx(float NdotH, float roughness)
{
	return PR_1_PI / (roughness * roughness * NdotH * NdotH * NdotH * 4);
}

inline Vector3f sample_ndf_ggx(float u0, float u1, float roughnessX, float roughnessY, float& pdf)
{
	float cosTheta, sinTheta; // V samples
	float cosPhi, sinPhi;	 // U samples

	float phi = std::atan(roughnessY / roughnessX * std::tan(PR_PI + 2 * PR_PI * u0)) + PR_PI * std::floor(2 * u0 + 0.5f);
	sinPhi	= std::sin(phi);
	cosPhi	= std::cos(phi);

	float f1	 = cosPhi / roughnessX;
	float f2	 = sinPhi / roughnessY;
	float alpha2 = 1 / (f1 * f1 + f2 * f2);
	float t2	 = alpha2 * u1 / (1 - u1);
	cosTheta	 = std::max(0.001f, 1.0f / std::sqrt(1 + t2));
	sinTheta	 = std::sqrt(1 - cosTheta * cosTheta);

	float s = 1 + t2 / alpha2;
	pdf		= PR_1_PI / (roughnessX * roughnessY * cosTheta * cosTheta * cosTheta * s * s);
	return Spherical::cartesian(sinTheta, cosTheta, sinPhi, cosPhi);
}

inline float pdf_ggx(float NdotH, float roughnessX, float roughnessY)
{
	return PR_1_PI / (roughnessX * roughnessY * NdotH * NdotH * NdotH * 4);
}

// Based on:
// Journal of Computer Graphics Techniques Vol. 7, No. 4, 2018 http://jcgt.org.
// Sampling the GGX Distribution of Visible Normals. Eric Heitz

inline float pdf_ggx_vndf(float NdotV, float VdotX, float VdotY,
						  float VdotH, float NdotH, float HdotX, float HdotY,
						  float roughnessX, float roughnessY)
{
	const float Dv = g_1_smith_nopt(NdotV, VdotX, VdotY, roughnessX, roughnessY) * VdotH * ndf_ggx(NdotH, HdotX, HdotY, roughnessX, roughnessY);
	return Dv / NdotV;
}

// nV is in sample space!
inline Vector3f sample_ggx_vndf(float u0, float u1,
								const Vector3f& nV,
								float roughnessX, float roughnessY, float& pdf)
{
	// Section 3.2: transforming the view direction to the hemisphere configuration
	const Vector3f Vh = Vector3f(roughnessX * nV(0), roughnessY * nV(1), nV(2)).normalized();

	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	float lensq = Vh(0) * Vh(0) + Vh(1) * Vh(1);
	Vector3f T1 = lensq > PR_EPSILON ? Vector3f(-Vh(1), Vh(0), 0) / std::sqrt(lensq) : Vector3f(1, 0, 0);
	Vector3f T2 = Vh.cross(T1);

	// Section 4.2: parameterization of the projected area
	float r   = std::sqrt(u0);
	float phi = 2.0f * PR_PI * u1;
	float t1  = r * std::cos(phi);
	float t2  = r * std::sin(phi);
	float s   = 0.5f * (1.0f + Vh(2));
	t2		  = (1.0f - s) * std::sqrt(1.0f - t1 * t1) + s * t2;

	// Section 4.3: reprojection onto hemisphere
	Vector3f Nh = t1 * T1 + t2 * T2 + std::sqrt(std::max(0.0f, 1.0f - t1 * t1 - t2 * t2)) * Vh;

	// Section 3.4: transforming the normal back to the ellipsoid configuration
	Vector3f H = Vector3f(roughnessX * Nh(0), roughnessY * Nh(1), std::max(0.0f, Nh(2))).normalized();

	pdf = pdf_ggx_vndf(nV(2), nV(0), nV(1), std::max(0.0f, nV.dot(H)),
					   H(2), H(0), H(1), roughnessX, roughnessY);
	return H;
}

//////////////////////////////
// Other kinds

/* Ward anisotropic distribution */
inline float ward(float alphaX, float alphaY,
				  float NdotV, float NdotL, float NdotH,
				  float HdotX, float HdotY)
{
	const float kx = HdotX / alphaX;
	const float ky = HdotY / alphaY;
	return std::exp(-2 * (kx * kx + ky * ky) / (1 + NdotH))
		   * std::sqrt(NdotL / NdotV)
		   / (4 * PR_PI * alphaX * alphaY);
}
} // namespace Microfacet
} // namespace PR
