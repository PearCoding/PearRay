#pragma once

#include "math/ShadingVector.h"
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
	const float k = roughness * std::sqrt(PR_INV_PI * 2.0f);
	return NdotK / std::fma(NdotK, 1 - k, k);
}

inline float g_1_walter(float NdotK, float roughness)
{
	const float a = roughness * std::sqrt(1 - NdotK * NdotK) / NdotK;
	return a >= 1.6f
			   ? 1.0f
			   : (3.535f * a + 2.181f * a * a) / (1 + 2.276f * a + 2.577f * a * a);
}

// 1/(4*NdotV*NdotL) already multiplied out
inline float g_1_smith_opt(float NdotK, float roughness)
{
	const float a = roughness * roughness;
	const float b = NdotK * NdotK;
	return 1.0f / (NdotK + std::sqrt(a + b - a * b));
}

// 1/(4*NdotV*NdotL) already multiplied out
inline float g_1_smith_opt(float NdotK, float KdotX, float KdotY, float roughnessX, float roughnessY)
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

// 1/(4*NdotV*NdotL) is not multiplied in
inline float g_1_smith(const ShadingVector& K, float roughness)
{
	const float a = roughness * roughness;
	const float b = K.tan2Theta();

	const float denom = 1 + std::sqrt(1 + a * b);
	return 2.0f / denom;
}

inline float g_1_smith(const ShadingVector& K, float roughnessX, float roughnessY)
{
	const float ax2 = K.sin2Phi() * roughnessX * roughnessX;
	const float ay2 = K.cos2Phi() * roughnessY * roughnessY;
	const float b	= K.tan2Theta();

	const float denom = 1 + std::sqrt(1 + (ax2 + ay2) * b);
	return 2.0f / denom;
}

/////////////////////////////////
// Distribution Terms

/* Normalized Beckmann distribution */
inline float ndf_beckmann(float NdotH, float roughness)
{
	const float k = NdotH * NdotH;
	const float m = roughness * roughness;
	return PR_INV_PI * std::exp((k - 1.0f) / (k * m)) / (k * k * m);
}

inline float ndf_blinn(float NdotH, float roughness)
{
	const float alpha2 = 1.0f / (roughness * roughness);
	return std::pow(NdotH, 2.0f * alpha2 - 2.0f) * alpha2 * PR_INV_PI;
}

inline float ndf_ggx(float NdotH, float roughness)
{
	const float alpha2 = roughness * roughness;
	const float dot2   = NdotH * NdotH;
	const float inv	   = dot2 * (alpha2 - 1) + 1;
	const float inv_t2 = inv * inv;

	if (inv_t2 <= PR_EPSILON)
		return 0.0f;
	else
		return alpha2 * PR_INV_PI / inv_t2;
}

inline float ndf_ggx(const ShadingVector& H, float roughnessX, float roughnessY)
{
	const float t = H.sin2Phi() / (roughnessX * roughnessX)
					+ H.cos2Phi() / (roughnessY * roughnessY)
					+ H.cos2Theta();
	const float denom = roughnessX * roughnessY * t * t;
	if (denom <= PR_EPSILON)
		return 0.0f;
	else
		return PR_INV_PI / denom;
}

////////////////////////////////
// Distribution Terms Sampling
inline Vector3f sample_ndf_beckmann(float u0, float u1, float roughness, float& pdf)
{
	float cosTheta, sinTheta; // V samples
	float cosPhi, sinPhi;	  // U samples
	float t	 = 1 / (1 - roughness * roughness * std::log(1 - u1));
	cosTheta = std::sqrt(t);
	sinTheta = std::sqrt(1 - cosTheta * cosTheta);
	sinPhi	 = std::sin(2 * PR_PI * u0);
	cosPhi	 = std::cos(2 * PR_PI * u0);

	if (roughness <= PR_EPSILON)
		pdf = 1;
	else
		pdf = PR_INV_PI / (roughness * roughness * cosTheta * cosTheta * cosTheta) * (1 - u1);

	return Spherical::cartesian(sinTheta, cosTheta, sinPhi, cosPhi);
}

inline float pdf_beckmann(float NdotH, float roughness)
{
	const float m = roughness * roughness;
	return PR_INV_PI / (NdotH * NdotH * NdotH * m);
}

inline Vector3f sample_ndf_blinn(float u0, float u1, float roughness, float& pdf)
{
	float cosTheta, sinTheta; // V samples
	float cosPhi, sinPhi;	  // U samples
	if (roughness <= PR_EPSILON)
		cosTheta = 1;
	else
		cosTheta = std::pow(1 - u1, 1 / (2 * roughness * roughness));

	sinTheta = std::sqrt(1 - cosTheta * cosTheta);
	sinPhi	 = std::sin(2 * PR_PI * u0);
	cosPhi	 = std::cos(2 * PR_PI * u0);

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
	float cosPhi, sinPhi;	  // U samples

	float alpha2 = roughness * roughness;
	float t2	 = alpha2 * u1 / (1 - u1);
	cosTheta	 = std::max(0.001f, 1.0f / std::sqrt(1 + t2));
	sinTheta	 = std::sqrt(1 - cosTheta * cosTheta);
	sinPhi		 = std::sin(2 * PR_PI * u0);
	cosPhi		 = std::cos(2 * PR_PI * u0);

	float s = 1 + t2 / alpha2;
	pdf		= PR_INV_PI / (alpha2 * cosTheta * cosTheta * cosTheta * s * s);
	return Spherical::cartesian(sinTheta, cosTheta, sinPhi, cosPhi);
}

inline float pdf_ggx(float NdotH, float roughness)
{
	return ndf_ggx(NdotH, roughness) * NdotH;
}

inline Vector3f sample_ndf_ggx(float u0, float u1, float roughnessX, float roughnessY, float& pdf)
{
	float cosTheta, sinTheta; // V samples
	float cosPhi, sinPhi;	  // U samples

	float phi = std::atan(roughnessY / roughnessX * std::tan(PR_PI + 2 * PR_PI * u0)) + PR_PI * std::floor(2 * u0 + 0.5f);
	sinPhi	  = std::sin(phi);
	cosPhi	  = std::cos(phi);

	float f1	 = cosPhi / roughnessX;
	float f2	 = sinPhi / roughnessY;
	float alpha2 = 1 / (f1 * f1 + f2 * f2);
	float t2	 = alpha2 * u1 / (1 - u1);
	cosTheta	 = std::max(0.001f, 1.0f / std::sqrt(1 + t2));
	sinTheta	 = std::sqrt(1 - cosTheta * cosTheta);

	float s = 1 + t2 / alpha2;
	pdf		= PR_INV_PI / (roughnessX * roughnessY * cosTheta * cosTheta * cosTheta * s * s);
	return Spherical::cartesian(sinTheta, cosTheta, sinPhi, cosPhi);
}

inline float pdf_ggx(const ShadingVector& H, float roughnessX, float roughnessY)
{
	return ndf_ggx(H, roughnessX, roughnessY) * H.absCosTheta();
}

// Based on:
// Journal of Computer Graphics Techniques Vol. 7, No. 4, 2018 http://jcgt.org.
// Sampling the GGX Distribution of Visible Normals. Eric Heitz

inline float pdf_ggx_vndf(const ShadingVector& V, const ShadingVector& H,
						  float roughnessX, float roughnessY)
{
	return g_1_smith(V, roughnessX, roughnessY) * std::abs(V.dot(H)) * ndf_ggx(H, roughnessX, roughnessY) /* V.absCosTheta()*/;
}

// nV is in sample space!
inline Vector3f sample_vndf_ggx(float u0, float u1,
								const Vector3f& nV,
								float roughnessX, float roughnessY, float& pdf)
{
	// Section 3.2: transforming the view direction to the hemisphere configuration
	const ShadingVector Vh = Vector3f(roughnessX * nV(0), roughnessY * nV(1), nV(2)).normalized();

#if 1
	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	const float lensq = sumProd(Vh(0), Vh(0), Vh(1), Vh(1));
	const Vector3f T1 = lensq > PR_EPSILON ? Vector3f(-Vh(1), Vh(0), 0) / std::sqrt(lensq) : Vector3f(1, 0, 0);
	const Vector3f T2 = ((Vector3f)Vh).cross(T1);

	// Section 4.2: parameterization of the projected area
	const float r	= std::sqrt(u0);
	const float phi = 2.0f * PR_PI * u1;
	const float t1	= r * std::cos(phi);
	float t2		= r * std::sin(phi);
	const float s	= 0.5f * (1.0f + Vh(2));
	t2				= (1.0f - s) * std::sqrt(1.0f - t1 * t1) + s * t2;

	// Section 4.3: reprojection onto hemisphere
	const Vector3f Nh = t1 * T1 + t2 * T2 + std::sqrt(std::max(0.0f, 1.0f + diffProd(-t1, t1, t2, t2))) * (Vector3f)Vh;
#else // Using the slope based approach
	const float tanTheta = Vh.tanTheta();
	const float a		 = 1 / tanTheta;
	const float g1		 = 2 / (1 + std::sqrt(1.f + 1.f / (a * a)));

	// sample slope_x
	const float A = 2 * u0 / g1 - 1;
	float C		  = 1.f / (A * A - 1.f);
	if (C > 1e10)
		C = 1e10;
	const float B		  = tanTheta;
	const float D		  = std::sqrt(std::max(0.0f, B * B * C * C - (A * A - B * B) * C));
	const float slope_x_1 = B * C - D;
	const float slope_x_2 = B * C + D;
	float slope_x		  = (A < 0 || slope_x_2 > a) ? slope_x_1 : slope_x_2;

	// sample slope_y
	float S;
	if (u1 > 0.5f) {
		S  = 1.0f;
		u1 = 2.0f * (u1 - 0.5f);
	} else {
		S  = -1.0f;
		u1 = 2.0f * (0.5f - u1);
	}
	float z		  = (u1 * (u1 * (u1 * 0.27385f - 0.73369f) + 0.46341f)) / (u1 * (u1 * (u1 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
	float slope_y = S * z * std::sqrt(1.f + slope_x * slope_x);

	float tmp = Vh.cosPhi() * slope_x - Vh.sinPhi() * slope_y;
	slope_y	  = Vh.sinPhi() * slope_x + Vh.cosPhi() * slope_y;
	slope_x	  = tmp;

	// 4. unstretch
	const Vector3f Nh = Vector3f(-slope_x, -slope_y, 1);
#endif

	// Section 3.4: transforming the normal back to the ellipsoid configuration
	const Vector3f H = Vector3f(roughnessX * Nh(0), roughnessY * Nh(1), std::max(0.0f, Nh(2))).normalized();

	pdf = pdf_ggx_vndf(nV, H, roughnessX, roughnessY);
	return H;
}

//////////////////////////////
// Other kinds

/// Ward anisotropic distribution
/// Ward brdf is only defined if NdotV and NdotL > 0
inline float ward(float alphaX, float alphaY,
				  float NdotV, float NdotL, float NdotH,
				  float HdotX, float HdotY)
{
	const float kx = HdotX / alphaX;
	const float ky = HdotY / alphaY;
	return std::exp(-2 * sumProd(kx, kx, ky, ky) / (1 + NdotH)) / (std::sqrt(NdotL * NdotV) * 4 * PR_PI * alphaX * alphaY);
}

/// Ward isotropic distribution
/// Ward brdf is only defined if NdotV and NdotL > 0
inline float ward(float alpha,
				  float NdotV, float NdotL, float NdotH)
{
	const float k = alpha * alpha;
	return std::exp(-2 / (k * (1 + NdotH))) / (std::sqrt(NdotL * NdotV) * 4 * PR_PI * k);
}
} // namespace Microfacet
} // namespace PR
