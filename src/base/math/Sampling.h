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

	pdf = PR_1_PI * 0.25f;

	return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi), z);
}

inline float sphere_pdf()
{
	return PR_1_PI * 0.25f;
}

// Orientation +Z
inline Vector3f hemi(float u1, float u2, float& pdf)
{
	float sinTheta = std::sqrt(std::max(0.0f, 1.0f - u1 * u1));
	float phi	   = 2 * PR_PI * u2;

	pdf = PR_1_PI * 0.5f;

	return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi), u1);
}

inline float hemi_pdf()
{
	return PR_1_PI * 0.5f;
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

	pdf = cosPhi * PR_1_PI;

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

	pdf = (m + 1.0f) * std::pow(cosPhi, m) * 0.5f * PR_1_PI;

	return Vector3f(x, y, cosPhi * norm);
}

inline float cos_hemi_pdf(float NdotL)
{
	return NdotL * PR_1_PI;
}

inline float cos_hemi_pdf(float NdotL, float m)
{
	return (m + 1.0f) * std::pow(NdotL, m) * 0.5f * PR_1_PI;
}

// Uniform
// Returns barycentric coordinates
inline Vector2f triangle(float u1, float u2)
{
	// Simplex method
	return u1 < u2 ? Vector2f(u1, u2 - u1) : Vector2f(u2, u1 - u2);
}

// Uniform
inline Vector2f concentric_disk(float u1, float u2)
{
	Vector2f off = 2.0f * Vector2f(u1, u2) - Vector2f(1, 1);
	if (off(0) == 0 && off(1) == 0)
		return Vector2f::Zero();

	float theta, r;
	if (std::abs(off(0)) > std::abs(off(1))) {
		r	  = off(0);
		theta = 4 * PR_PI * (off(1) / off(0));
	} else {
		r	  = off(1);
		theta = 2 * PR_PI - 4 * PR_PI * (off(0) / off(1));
	}
	
	return r * Vector2f(std::cos(theta), std::sin(theta));
}
} // namespace Sampling
} // namespace PR
