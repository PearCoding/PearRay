#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class PR_LIB Face
	{
	public:
		Face();
		~Face();

		PM::vec3 V1;
		PM::vec3 V2;
		PM::vec3 V3;

		PM::vec3 N1;
		PM::vec3 N2;
		PM::vec3 N3;

		PM::vec2 UV1;
		PM::vec2 UV2;
		PM::vec2 UV3;

		inline void interpolate(float u, float v, PM::vec3& vec, PM::vec3& norm, PM::vec2& uv) const
		{
			vec = PM::pm_Add(PM::pm_Scale(V2, u),
				PM::pm_Add(PM::pm_Scale(V3, v), PM::pm_Scale(V1, 1 - u - v)));

			norm = PM::pm_Add(PM::pm_Scale(N2, u),
				PM::pm_Add(PM::pm_Scale(N3, v), PM::pm_Scale(N1, 1 - u - v)));

			uv = PM::pm_Add(PM::pm_Scale(UV2, u),
				PM::pm_Add(PM::pm_Scale(UV3, v), PM::pm_Scale(UV1, 1 - u - v)));
		}
	};
}