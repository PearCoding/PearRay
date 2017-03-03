#pragma once

#include "PR_Config.h"
#include "PearMath.h"

namespace PR
{
	class Material;
	class PR_LIB Face
	{
	public:
		Face() :
			V{PM::pm_Zero3D(), PM::pm_Zero3D(), PM::pm_Zero3D()},
			N{PM::pm_Zero3D(), PM::pm_Zero3D(), PM::pm_Zero3D()},
			UV{PM::pm_Zero2D(), PM::pm_Zero2D(), PM::pm_Zero2D()},
			MaterialSlot(0)
		{
		}

		PM::vec3 V[3];
		PM::vec3 N[3];
		PM::vec2 UV[3];

		inline void interpolate(float u, float v, PM::vec3& vec, PM::vec3& norm, PM::vec2& uv) const
		{
			vec = PM::pm_Add(PM::pm_Scale(V[1], u),
				PM::pm_Add(PM::pm_Scale(V[2], v), PM::pm_Scale(V[0], 1 - u - v)));

			norm = PM::pm_Normalize(PM::pm_Add(PM::pm_Scale(N[1], u),
				PM::pm_Add(PM::pm_Scale(N[2], v), PM::pm_Scale(N[0], 1 - u - v))));

			uv = PM::pm_Add(PM::pm_Scale(UV[1], u),
				PM::pm_Add(PM::pm_Scale(UV[2], v), PM::pm_Scale(UV[0], 1 - u - v)));
		}

		uint32 MaterialSlot;
	};
}
