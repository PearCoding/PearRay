#pragma once

#include "Config.h"
#include "PearMath.h"
#include "sampler/Sampler.h"

namespace PR
{
	class PR_LIB Projection
	{
		PR_CLASS_NON_COPYABLE(Projection);
	public:
		// Map [0, 1] uniformly to [min, max] as integers! (max is included)
		static inline int map(float u, int min, int max)
		{
			return PM::pm_MinT<int>(max - min, static_cast<int>(u*(max - min + 1))) + min;
		}

		static inline float stratified(float u, int index, int groups, float min = 0, float max = 1)
		{
			float range = (max - min) / groups;
			return min + u*range + index*range;
		}

		// Align v on N
		static inline PM::vec3 align(const PM::vec3& N, const PM::vec3& V)
		{
			return align(N, V, PM::pm_Set(0, 0, 1));
		}

		static inline PM::vec3 align(const PM::vec3& N, const PM::vec3& V, const PM::vec3& axis)
		{
			const float dot = PM::pm_Dot3D(N, axis);
			if (dot + 1 < PM_EPSILON)
				return PM::pm_Negate(V);
			else if (dot < 1)
				return PM::pm_RotateWithQuat(PM::pm_RotateFromTo(axis, N), V);

			return V;
		}

		// N Orientation Z+
		static inline void tangent_frame(const PM::vec3& N, PM::vec3& T, PM::vec3& B)
		{
			PM::vec3 t = std::abs(PM::pm_GetX(N)) > 0.99f ? PM::pm_Set(0, 1, 0) : PM::pm_Set(1, 0, 0);
			T = PM::pm_Normalize3D(PM::pm_Cross3D(N, t));
			B = PM::pm_Cross3D(N, T);
		}

		static inline PM::vec3 tangent_align(const PM::vec3& N, const PM::vec3& V)
		{
			PM::vec3 X, Y;
			tangent_frame(N, X, Y);
			return tangent_align(N, X, Y, V);
		}

		static inline PM::vec3 tangent_align(const PM::vec3& N, const PM::vec3& T, const PM::vec3& B, const PM::vec3& V)
		{
			return PM::pm_Add(PM::pm_Add(PM::pm_Scale(N, PM::pm_GetZ(V)), PM::pm_Scale(T, PM::pm_GetY(V))),
				PM::pm_Scale(B, PM::pm_GetX(V)));
		}

		static inline PM::vec2 sphereUV(const PM::vec3& V)
		{
			float u = 0.5f + std::atan2(PM::pm_GetZ(V), PM::pm_GetX(V)) * PM_INV_PI_F * 0.5f;
			float v = 0.5f - std::asin(-PM::pm_GetY(V)) * PM_INV_PI_F;
			return PM::pm_Set(u, v);
		}

		// Projections 
		// Uniform [0, 1]
		static inline PM::vec3 sphere(float u1, float u2, float& pdf)
		{
			const float t1 = PM_2_PI_F * u1;
			const float t2 = 2 * std::sqrt(u2 + u2*u2);
			const float norm = 1.0f/std::sqrt(1+8*u2*u2);

			float thCos, thSin;
			PM::pm_SinCosT(t1, thSin, thCos);

			const float x = t2 * thCos;
			const float y = t2 * thSin;
			const float z = 1 - 2.0f * u2;

			pdf = PM_INV_PI_F * 0.25f;
			
			return PM::pm_Set(x*norm,y*norm,z*norm);
		}

		static inline float sphere_pdf()
		{
			return PM_INV_PI_F * 0.25f;
		}

		// theta [0, PI]
		// phi [0, 2*PI]
		static inline PM::vec3 sphere_coord(float theta, float phi)
		{
			float thCos, thSin;
			PM::pm_SinCosT(theta, thSin, thCos);

			float phCos, phSin;
			PM::pm_SinCosT(phi, phSin, phCos);

			return PM::pm_Set(thSin * phCos,
				thSin * phSin,
				thCos);
		}

		// Cosine weighted
		// Orientation +Z
		static inline PM::vec3 cos_hemi(float u1, float u2, float& pdf)
		{
			const float cosPhi = std::sqrt(u1);
			const float sinPhi = std::sqrt(1 - u1);// Faster?
			const float theta = PM_2_PI_F * u2;

			float thCos, thSin;
			PM::pm_SinCosT(theta, thSin, thCos);

			const float x = sinPhi * thCos;
			const float y = sinPhi * thSin;

			pdf = cosPhi * PM_INV_PI_F;
			
			return PM::pm_Set(x, y, cosPhi);
		}

		static inline PM::vec3 cos_hemi(float u1, float u2, float m, float& pdf)
		{
			const float cosPhi = std::pow(u1, 1 / (m + 1.0f));
			const float sinPhi = std::sqrt(1 - cosPhi*cosPhi);
			const float theta = PM_2_PI_F * u2;
			const float norm = 1.0f/std::sqrt(1-u1+cosPhi*cosPhi);

			float thCos, thSin;
			PM::pm_SinCosT(theta, thSin, thCos);

			const float x = sinPhi * thCos * norm;
			const float y = sinPhi * thSin * norm;

			pdf = (m + 1.0f) * std::pow(cosPhi, m) * 0.5f * PM_INV_PI_F;

			return PM::pm_Set(x, y, cosPhi * norm);
		}

		static inline float cos_hemi_pdf(float NdotL)
		{
			return PM_INV_PI_F * NdotL;
		}

		static inline float cos_hemi_pdf(float NdotL, float m)
		{
			return (m + 1.0f) * std::pow(NdotL, m) * 0.5f * PM_INV_PI_F;
		}

		// Uniform
		// Returns barycentric coordinates
		static inline PM::vec2 triangle(float u1, float u2)
		{
			const float t = std::sqrt(u1);
			return PM::pm_Set(1 - t, u2 * t);
		}
	};
}