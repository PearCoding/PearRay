#pragma once

#include "Spherical.h"

namespace PR {
namespace Projection {
// Map [0, 1] uniformly to [min, max] as integers! (max is included)
template <typename T>
inline T map(float u, T min, T max)
{
	return std::min<T>(max - min, static_cast<T>(u * (max - min + 1))) + min;
}

inline float stratified(float u, int index, int groups, float min = 0, float max = 1)
{
	float range = (max - min) / groups;
	return min + u * range + index * range;
}

// Projections
// Uniform [0, 1]
inline Vector3f sphere(float u1, float u2, float& pdf)
{
	const float t1   = 2 * PR_PI * u1;
	const float t2   = 2 * std::sqrt(u2 + u2 * u2);
	const float norm = 1.0f / std::sqrt(1 + 8 * u2 * u2);

	const float thSin = std::sin(t1);
	const float thCos = std::cos(t1);

	const float x = t2 * thCos;
	const float y = t2 * thSin;
	const float z = 1 - 2.0f * u2;

	pdf = PR_1_PI * 0.25f;

	return Vector3f(x * norm, y * norm, z * norm);
}

inline float sphere_pdf()
{
	return PR_1_PI * 0.25f;
}

// Orientation +Z
inline Vector3f hemi(float u1, float u2, float& pdf)
{
	pdf = PR_1_PI * 0.5f;
	return Spherical::cartesian(u1 * PR_PI * 0.5f, u2 * 2 * PR_PI);
}

inline float hemi_pdf()
{
	return PR_1_PI * 0.25f;
}

// Cosine weighted
// Orientation +Z
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
} // namespace Projection
} // namespace PR
