#pragma once

#include "PR_Config.h"

namespace PR {
namespace Sampling {
// Uniform [0, 1]
inline Vector3f sphere(float u1, float u2, float& pdf)
{
	const float z		 = 1 - 2 * u1;
	const float sinTheta = std::sqrt(std::max(0.0f, 1.0f - z * z));
	const float phi		 = 2 * PR_PI * u2;

	pdf = PR_INV_PI * 0.25f;

	return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi), z);
}

inline float sphere_pdf()
{
	return PR_INV_PI * 0.25f;
}

// Orientation +Z
inline Vector3f hemi(float u1, float u2, float& pdf)
{
	float sinTheta = std::sqrt(std::max(0.0f, 1.0f - u1 * u1));
	float phi	   = 2 * PR_PI * u2;

	pdf = PR_INV_PI * 0.5f;

	return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi), u1);
}

inline float hemi_pdf()
{
	return PR_INV_PI * 0.5f;
}

// Cosine weighted
// Orientation +Z (shading space)
inline Vector3f cos_hemi(float u1, float u2, float& pdf)
{
	const float cosPhi = std::sqrt(u1);
	const float sinPhi = std::sqrt(1 - u1); // Faster?
	const float theta  = 2 * PR_PI * u2;

	const float thSin = std::sin(theta);
	const float thCos = std::cos(theta);

	const float x = sinPhi * thCos;
	const float y = sinPhi * thSin;

	pdf = cosPhi * PR_INV_PI;

	return Vector3f(x, y, cosPhi);
}

inline Vector3f cos_hemi(float u1, float u2, float m, float& pdf)
{
	const float cosPhi = std::pow(u1, 1 / (m + 1.0f));
	const float sinPhi = std::sqrt(1 - cosPhi * cosPhi);
	const float theta  = 2 * PR_PI * u2;
	const float norm   = 1.0f / std::sqrt(1 - u1 + cosPhi * cosPhi);

	const float thSin = std::sin(theta);
	const float thCos = std::cos(theta);

	const float x = sinPhi * thCos * norm;
	const float y = sinPhi * thSin * norm;

	pdf = (m + 1.0f) * std::pow(cosPhi, m) * PR_INV_2_PI;

	return Vector3f(x, y, cosPhi * norm);
}

inline float cos_hemi_pdf(float NdotL)
{
	return NdotL * PR_INV_PI;
}

inline float cos_hemi_pdf(float NdotL, float m)
{
	return (m + 1.0f) * std::pow(NdotL, m) * PR_INV_2_PI;
}

// Uniform
// Returns barycentric coordinates
inline Vector2f triangle(float u1, float u2)
{
	// Simplex method
	return u1 < u2 ? Vector2f(u1, u2 - u1) : Vector2f(u2, u1 - u2);
}

// Uniform cone
// Orientation +Z (shading space)
inline Vector3f uniform_cone(float u1, float u2, float cos_theta_max)
{
	float cosTheta = std::fma(u1, cos_theta_max, 1 - u1); // Lerp between cos_theta_max and 1
	float sinTheta = std::sqrt(std::max(0.0f, diffProd<float>(1, 1, cosTheta, cosTheta)));
	float phi	   = 2 * PR_PI * u2;
	return Vector3f(std::cos(phi) * sinTheta, std::sin(phi) * sinTheta, cosTheta);
}

// In solid angle
inline float uniform_cone_pdf(float cos_theta_max)
{
	return PR_INV_2_PI / (1 - cos_theta_max);
}

} // namespace Sampling
} // namespace PR
