#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class Material;
	class PR_LIB Face
	{
	public:
		Face() :
			V{PM::pm_Set(0, 0, 0, 1), PM::pm_Set(0, 0, 0, 1), PM::pm_Set(0, 0, 0, 1)},
			N{PM::pm_Zero(), PM::pm_Zero(), PM::pm_Zero()},
			UV{PM::pm_Zero(), PM::pm_Zero(), PM::pm_Zero()},
			Mat(nullptr)
		{
		}

		PM::vec3 V[3];
		PM::vec3 N[3];
		PM::vec2 UV[3];

		inline void interpolate(float u, float v, PM::vec3& vec, PM::vec3& norm, PM::vec2& uv) const
		{
			vec = PM::pm_Add(PM::pm_Scale(V[1], u),
				PM::pm_Add(PM::pm_Scale(V[2], v), PM::pm_Scale(V[0], 1 - u - v)));

			norm = PM::pm_Normalize3D(PM::pm_Add(PM::pm_Scale(N[1], u),
				PM::pm_Add(PM::pm_Scale(N[2], v), PM::pm_Scale(N[0], 1 - u - v))));

			uv = PM::pm_Add(PM::pm_Scale(UV[1], u),
				PM::pm_Add(PM::pm_Scale(UV[2], v), PM::pm_Scale(UV[0], 1 - u - v)));
		}

		Material* Mat;
	};
}