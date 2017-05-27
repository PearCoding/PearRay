#pragma once

#include "sampler/Sampler.h"

namespace PR {
class PR_LIB Projection {
	PR_CLASS_NON_CONSTRUCTABLE(Projection);

public:
	// Map [0, 1] uniformly to [min, max] as integers! (max is included)
	static inline int map(float u, int min, int max)
	{
		return std::min<int>(max - min, static_cast<int>(u * (max - min + 1))) + min;
	}

	static inline float stratified(float u, int index, int groups, float min = 0, float max = 1)
	{
		float range = (max - min) / groups;
		return min + u * range + index * range;
	}

	// Align v on N
	static inline Eigen::Vector3f align(const Eigen::Vector3f& N, const Eigen::Vector3f& V)
	{
		return align(N, V, Eigen::Vector3f(0, 0, 1));
	}

	static inline Eigen::Vector3f align(const Eigen::Vector3f& N, const Eigen::Vector3f& V, const Eigen::Vector3f& axis)
	{
		const float dot = N.dot(axis);
		if (dot + 1 < PR_EPSILON)
			return -V;
		else if (dot < 1)
			return Eigen::Quaternionf::FromTwoVectors(axis, N) * V;

		return V;
	}

	// N Orientation Z+
	static inline void tangent_frame(const Eigen::Vector3f& N, Eigen::Vector3f& T, Eigen::Vector3f& B)
	{
		Eigen::Vector3f t = std::abs(N(0)) > 0.99f ? Eigen::Vector3f(0, 1, 0) : Eigen::Vector3f(1, 0, 0);
		T				  = N.cross(t).normalized();
		B				  = N.cross(T).normalized();
	}

	static inline Eigen::Vector3f tangent_align(const Eigen::Vector3f& N, const Eigen::Vector3f& V)
	{
		Eigen::Vector3f X, Y;
		tangent_frame(N, X, Y);
		return tangent_align(N, X, Y, V);
	}

	static inline Eigen::Vector3f tangent_align(const Eigen::Vector3f& N, const Eigen::Vector3f& Nx, const Eigen::Vector3f& Ny, const Eigen::Vector3f& V)
	{
		return N * V(2) + Ny * V(1) + Nx * V(0);
	}

	static inline Eigen::Vector2f sphereUV(const Eigen::Vector3f& V)
	{
		float u = 0.5f + std::atan2(V(2), V(0)) * PR_1_PI * 0.5f;
		float v = 0.5f - std::asin(-V(1)) * PR_1_PI;
		return Eigen::Vector2f(u, v);
	}

	// Projections
	// Uniform [0, 1]
	static inline Eigen::Vector3f sphere(float u1, float u2, float& pdf)
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

		return Eigen::Vector3f(x * norm, y * norm, z * norm);
	}

	static inline float sphere_pdf()
	{
		return PR_1_PI * 0.25f;
	}

	// theta [0, PI]
	// phi [0, 2*PI]
	static inline Eigen::Vector3f sphere_coord(float theta, float phi)
	{
		const float thSin = std::sin(theta);
		const float thCos = std::cos(theta);

		const float phSin = std::sin(phi);
		const float phCos = std::cos(phi);

		return Eigen::Vector3f(thSin * phCos,
							   thSin * phSin,
							   thCos);
	}

	// Orientation +Z
	static inline Eigen::Vector3f hemi(float u1, float u2, float& pdf)
	{
		const float thSin = std::sin(u1 * PR_PI * 0.5f);
		const float thCos = std::cos(u1 * PR_PI * 0.5f);

		const float phSin = std::sin(u2 * 2 * PR_PI);
		const float phCos = std::cos(u2 * 2 * PR_PI);

		pdf = 1;

		return Eigen::Vector3f(thSin * phCos,
							   thSin * phSin,
							   thCos);
	}

	// Cosine weighted
	// Orientation +Z
	static inline Eigen::Vector3f cos_hemi(float u1, float u2, float& pdf)
	{
		const float cosPhi = std::sqrt(u1);
		const float sinPhi = std::sqrt(1 - u1); // Faster?
		const float theta  = 2 * PR_PI * u2;

		const float thSin = std::sin(theta);
		const float thCos = std::cos(theta);

		const float x = sinPhi * thCos;
		const float y = sinPhi * thSin;

		pdf = cosPhi;

		return Eigen::Vector3f(x, y, cosPhi);
	}

	static inline Eigen::Vector3f cos_hemi(float u1, float u2, float m, float& pdf)
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

		return Eigen::Vector3f(x, y, cosPhi * norm);
	}

	static inline float cos_hemi_pdf(float NdotL)
	{
		return NdotL * 0.5f * PR_1_PI;
	}

	static inline float cos_hemi_pdf(float NdotL, float m)
	{
		return (m + 1.0f) * std::pow(NdotL, m) * 0.5f * PR_1_PI;
	}

	// Uniform
	// Returns barycentric coordinates
	static inline Eigen::Vector2f triangle(float u1, float u2)
	{
		// Simplex method
		return u1 < u2 ? Eigen::Vector2f(u1, u2 - u1) : Eigen::Vector2f(u2, u1 - u2);
	}
};
}
