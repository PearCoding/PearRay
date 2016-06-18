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

		// Projections 
		
		// Uniform [0, 1]
		static inline PM::vec3 sphere(float phi, float rho)
		{
			float sinPhi, cosPhi;
			float sinRho, cosRho;

			PM::pm_SinCosT(PM_PI_F * phi, sinPhi, cosPhi);
			PM::pm_SinCosT(PM_2_PI_F * rho, sinRho, cosRho);

			return PM::pm_Set(
				sinPhi*cosRho,
				sinPhi*sinRho,
				cosPhi);
		}

		// Not really uniform [0, 1]
		static inline PM::vec3 sphereFast(float dx, float dy, float dz)
		{
			return PM::pm_Normalize3D(PM::pm_Set(2 * dx - 1, 2 * dy - 1, 2 * dz - 1, 1));
		}

		static inline PM::vec3 sphereReject(Sampler& sampler)
		{
			for (uint32 i = 0; i < 1000000; ++i)
			{
				PM::vec3 d = PM::pm_Subtract(PM::pm_Scale(sampler.generate3D(i), 2.0f), PM::pm_Set(1,1,1));

				if (PM::pm_MagnitudeSqr3D(d) <= 1)
					return PM::pm_Normalize3D(d);
			}

			return PM::pm_Set(0,0,1);
		}

		// Uniform
		// Orientation +Z
		static inline PM::vec3 hemi(float u1, float u2)
		{
			const float r = std::sqrt(1 - u1*u1);
			const float theta = PM_2_PI_F * u2;

			float thCos, thSin;
			PM::pm_SinCosT(theta, thSin, thCos);

			const float x = r * thCos;
			const float y = r * thSin;

			return PM::pm_Normalize3D(PM::pm_Set(x, y, u1));
		}

		// Cosine weighted
		// Orientation +Z
		static inline PM::vec3 cos_hemi(float u1, float u2)
		{
			const float cosPhi = std::sqrt(1 - u1);
			const float sinPhi = std::sin(std::acos(cosPhi));// Faster?
			const float theta = PM_2_PI_F * u2;

			float thCos, thSin;
			PM::pm_SinCosT(theta, thSin, thCos);

			const float x = sinPhi * thCos;
			const float y = sinPhi * thSin;

			return PM::pm_Set(x, y, cosPhi);
		}

		static inline PM::vec3 cos_hemi(float u1, float u2, int m)
		{
			const float cosPhi = std::pow(1 - u1, 1 / (m + 1.0f));
			const float sinPhi = std::sin(std::acos(cosPhi));// Faster?
			const float theta = PM_2_PI_F * u2;

			float thCos, thSin;
			PM::pm_SinCosT(theta, thSin, thCos);

			const float x = sinPhi * thCos;
			const float y = sinPhi * thSin;

			return PM::pm_Set(x, y, cosPhi);
		}
	};
}