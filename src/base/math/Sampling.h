#pragma once

#include "PR_Config.h"

namespace PR {
namespace Sampling {
// Uniform [0, 1]
inline Vector3f sphere(float u1, float u2)
{
	const float cosTheta = 1 - 2 * u1;
	const float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
	const float phi		 = 2 * PR_PI * u2;

	return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
}

inline float sphere_pdf()
{
	return PR_INV_PI * 0.25f;
}

// Orientation +Z
inline Vector3f hemi(float u1, float u2)
{
	float sinTheta = std::sqrt(std::max(0.0f, 1.0f - u1 * u1));
	float phi	   = 2 * PR_PI * u2;

	return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi), u1);
}

inline float hemi_pdf()
{
	return PR_INV_PI * 0.5f;
}

// Cosine weighted
// Orientation +Z (shading space)
inline Vector3f cos_hemi(float u1, float u2)
{
	const float cosTheta = std::sqrt(u1);
	const float sinTheta = std::sqrt(1 - u1); // Faster?
	const float phi		 = 2 * PR_PI * u2;

	const float sinPhi = std::sin(phi);
	const float cosPhi = std::cos(phi);

	const float x = sinTheta * cosPhi;
	const float y = sinTheta * sinPhi;

	return Vector3f(x, y, cosTheta);
}

inline float cos_hemi_pdf(float NdotL)
{
	return NdotL * PR_INV_PI;
}

// Power cosine weighted
// Orientation +Z (shading space)
inline Vector3f cos_hemi(float u1, float u2, float m)
{
	const float cosTheta = std::pow(u1, 1 / (m + 1.0f));
	const float sinTheta = std::sqrt(1 - cosTheta * cosTheta);
	const float phi		 = 2 * PR_PI * u2;
	const float norm	 = 1.0f / std::sqrt(1 - u1 + cosTheta * cosTheta);

	const float sinPhi = std::sin(phi);
	const float cosPhi = std::cos(phi);

	const float x = sinTheta * cosPhi * norm;
	const float y = sinTheta * sinPhi * norm;

	return Vector3f(x, y, cosTheta * norm);
}

inline float cos_hemi_pdf(float NdotL, float m)
{
	return (m + 1.0f) * std::pow(NdotL, m) * PR_INV_2_PI;
}

// Uniform
// Sample barycentric point on the unit triangle
// Returns barycentric coordinates
inline Vector2f triangle(float u1, float u2)
{
	// Simplex method
	return u1 < u2 ? Vector2f(u1, u2 - u1) : Vector2f(u2, u1 - u2);
}

// Return the pdf based on area sampling of the unit triangle (A=1/2)
inline float triangle_pdf()
{
	constexpr float A = 0.5f;
	return 1 / A;
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
