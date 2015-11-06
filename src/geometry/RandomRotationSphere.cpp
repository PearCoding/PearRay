#include "RandomRotationSphere.h"

namespace PR
{
	// Tooo slow!?
	PM::vec3 RandomRotationSphere::create(const PM::vec3& normal, float sph, float eph, float srh, float erh)
	{
		float phi = sph + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (eph - sph)));
		float rho = srh + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (erh - srh)));

		float nph = std::acosf(PM::pm_GetZ(normal));
		float nrh = PM::pm_GetX(normal) != 0 ? std::atanf(PM::pm_GetY(normal)/PM::pm_GetX(normal)) : 0;// Really ok?

		float sinphi = std::sinf(nph + phi);
		return PM::pm_Set(
			sinphi*std::cos(nrh + rho),
			sinphi*std::sin(nrh + rho),
			std::cos(nph + phi));
	}
}