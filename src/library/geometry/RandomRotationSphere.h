#pragma once

#include "Config.h"
#include "PearMath.h"
#include "Random.h"

namespace PR
{
	class PR_LIB RandomRotationSphere
	{
		PR_CLASS_NON_COPYABLE(RandomRotationSphere);
	public:
		static inline PM::vec3 create(const PM::vec3& normal, float sph, float eph, float srh, float erh, Random& rand) {
			float nph = std::acosf(
				PM::pm_MaxT<float>(-1, PM::pm_MinT<float>(1, PM::pm_GetZ(normal))));
			float nrh = PM::pm_GetX(normal) != 0 ? std::atan2f(PM::pm_GetY(normal), PM::pm_GetX(normal)) : 0;

			return create(nph, nrh, sph, eph, srh, erh, rand);
		}

		static inline PM::vec3 create(float nph, float nrh, float sph, float eph, float srh, float erh, Random& rand)
		{
			float phi = sph + rand.getFloat() * (eph - sph);
			float rho = srh + rand.getFloat() * (erh - srh);

			nph += phi;
			nrh += rho;

			float sinphi = std::sinf(nph);
			return PM::pm_Set(
				sinphi*std::cos(nrh),
				sinphi*std::sin(nrh),
				std::cos(nph));
		}

		static inline PM::vec3 createFast(const PM::vec3& normal,
			float sx, float ex, float sy, float ey, float sz, float ez, Random& rand)
		{
			float dx = sx + rand.getFloat() * (ex - sx);
			float dy = sy + rand.getFloat() * (ey - sy);
			float dz = sz + rand.getFloat() * (ez - sz);
#
			PM::vec n = PM::pm_Normalize3D(PM::pm_Set(dx, dy, dz, 1));

			return PM::pm_Dot3D(normal, n) <= 0 ? PM::pm_Negate(n) : n;
		}
	};
}