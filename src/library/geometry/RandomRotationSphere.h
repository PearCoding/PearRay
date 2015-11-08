#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class PR_LIB RandomRotationSphere
	{
		PR_CLASS_NON_COPYABLE(RandomRotationSphere);
	public:
		static inline PM::vec3 create(const PM::vec3& normal, float sph, float eph, float srh, float erh) {
			float nph = std::acosf(PM::pm_GetZ(normal));
			float nrh = PM::pm_GetX(normal) != 0 ? std::atan2f(PM::pm_GetY(normal), PM::pm_GetX(normal)) : 0;

			return create(nph, nrh, sph, eph, srh, erh);
		}

		static inline PM::vec3 create(float nph, float nrh, float sph, float eph, float srh, float erh)
		{
			float phi = sph + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (eph - sph)));
			float rho = srh + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (erh - srh)));

			nph += phi;
			nrh += rho;

			float sinphi = std::sinf(nph);
			return PM::pm_Set(
				sinphi*std::cos(nrh),
				sinphi*std::sin(nrh),
				std::cos(nph));
		}

		static inline PM::vec3 createFast(const PM::vec3& normal, float sx, float ex, float sy, float ey, float sz, float ez)
		{
			float dx = sx + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (ex - sx)));
			float dy = sy + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (ey - sy)));
			float dz = sz + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (ez - sz)));

			return PM::pm_Normalize3D(PM::pm_Set(
				PM::pm_GetX(normal) + dx,
				PM::pm_GetY(normal) + dy,
				PM::pm_GetZ(normal) + dz));
		}
	};
}