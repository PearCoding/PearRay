#pragma once

#include "PR_Config.h"

namespace PR {
namespace Microfacet {
/* All functions are in the sampling space, with L and V!! are pointing out the surface.
 * This also means that NdotL, NdotV and NdotH can not be negative!
 */

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

	return alpha2 * PR_1_PI / inv_t2;
}

inline float ndf_ggx(float NdotH, float HdotX, float HdotY, float roughnessX, float roughnessY)
{
	const float t = HdotX * HdotX / (roughnessX * roughnessX)
					+ HdotY * HdotY / (roughnessY * roughnessY)
					+ NdotH * NdotH;
	return PR_1_PI / (roughnessX * roughnessY * t * t);
}

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
} // namespace Specular
} // namespace PR
