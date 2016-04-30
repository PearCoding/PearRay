#pragma once

#include "Config.h"
#include "PearMath.h"
#include "Random.h"

namespace PR
{
	class PR_LIB Sampler
	{
		PR_CLASS_NON_COPYABLE(Sampler);
	public:
		static inline float stratified(float u, int index, int groups, float min = 0, float max = 1)
		{
			float range = (max - min) / groups;
			return min + u*range + index*range;
		}

		// Align v on N
		static inline PM::vec3 align(const PM::vec3& N, const PM::vec3& v)
		{
			return align(N, v, PM::pm_Set(0, 0, 1));
		}

		static inline PM::vec3 align(const PM::vec3& N, const PM::vec3& v, const PM::vec3& axis)
		{
			float angle = std::acos(PM::pm_Dot3D(v, axis));
			if (std::abs(angle) > std::numeric_limits<float>::epsilon())
			{
				PM::vec3 rot_axis = PM::pm_Scale(PM::pm_Normalize3D(PM::pm_Cross3D(N, axis)), -angle);
				return PM::pm_RotateWithQuat(PM::pm_RotationQuatXYZ(rot_axis), v);
			}

			return v;
		}

		// Projections 
		
		// Uniform [0, 1]
		static inline PM::vec3 sphere(float phi, float rho)
		{
			float sinPhi, cosPhi;
			float sinRho, cosRho;

			PM::pm_SinCosT(PM_2_PI_F * phi, sinPhi, cosPhi);
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

		// Uniform
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
		static inline PM::vec3 cos_hemi(float u1, float u2)
		{
			const float r = std::sqrt(u1);
			const float theta = PM_2_PI_F * u2;

			float thCos, thSin;
			PM::pm_SinCosT(theta, thSin, thCos);

			const float x = r * thCos;
			const float y = r * thSin;

			return PM::pm_Normalize3D(PM::pm_Set(x, y, std::sqrt(PM::pm_MaxT(0.0f, 1 - u1))));
		}
	};
}