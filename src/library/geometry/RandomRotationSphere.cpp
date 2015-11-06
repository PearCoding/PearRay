#include "RandomRotationSphere.h"

namespace PR
{
	// Tooo slow!?
	PM::vec3 RandomRotationSphere::create(const PM::vec3& normal, float sph, float eph, float srh, float erh)
	{
		float phi = sph + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (eph - sph)));
		float rho = srh + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (erh - srh)));

		float nph = std::acosf(PM::pm_GetZ(normal)) + phi;
		float nrh = (PM::pm_GetX(normal) != 0 ? std::atanf(PM::pm_GetY(normal)/PM::pm_GetX(normal)) : 0) + rho;// Really ok?

		float sinphi = std::sinf(nph);
		return PM::pm_Set(
			sinphi*std::cos(nrh),
			sinphi*std::sin(nrh),
			std::cos(nph));
	}

	PM::vec3 RandomRotationSphere::createFast(const PM::vec3& normal, float sx, float ex, float sy, float ey, float sz, float ez)
	{
		float dx = sx+ static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (ex - sx)));
		float dy = sy + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (ey - sy)));
		float dz = sz + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (ez - sz)));

		return PM::pm_Normalize3D(PM::pm_Set(
			PM::pm_GetX(normal)+dx,
			PM::pm_GetY(normal) + dy,
			PM::pm_GetZ(normal) + dz));
	}
}