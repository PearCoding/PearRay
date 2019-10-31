#pragma once

#include "PR_Config.h"

namespace PR {
namespace Specular {
/* All functions are in the sampling space, with L and V!! are pointing out the surface.
 * This also means that NdotL, NdotV and NdotH can not be negative!
 */

/* Standard Beckmann distribution */
inline float beckmann(float NdotH, float roughness)
{
	if (NdotH < PR_EPSILON
		|| roughness < PR_EPSILON)
		return 0;

	const float k = NdotH * NdotH;
	const float m = roughness * roughness;
	return PR_1_PI * std::exp((k - 1) / (k * m)) / (k * k * m);
}

/* Ward anisotropic distribution */
inline float ward(float alphaX, float alphaY,
				  float NdotV, float NdotL, float NdotH,
				  float HdotX, float HdotY)
{
	if (alphaX < PR_EPSILON
		|| alphaY < PR_EPSILON
		|| NdotV < PR_EPSILON
		|| NdotL < PR_EPSILON
		|| NdotH < PR_EPSILON)
		return 0;

	const float kx = HdotX / alphaX;
	const float ky = HdotY / alphaY;
	return std::exp(-2 * (kx * kx + ky * ky) / (1 + NdotH))
		   * std::sqrt(NdotL / NdotV)
		   / (4 * PR_PI * alphaX * alphaY);
}
} // namespace Specular
} // namespace PR
