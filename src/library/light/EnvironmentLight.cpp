#include "EnvironmentLight.h"
#include "shader/ShaderClosure.h"
#include "shader/ShaderOutput.h"
#include "spectral/Spectrum.h"
#include "material/Material.h"

#include "math/Projection.h"

namespace PR
{
	EnvironmentLight::EnvironmentLight() :
		mMaterial(nullptr)
	{

	}

	EnvironmentLight::~EnvironmentLight()
	{
	}

	PM::vec3 EnvironmentLight::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		return Projection::tangent_align(point.N,
			Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), pdf));
	}

	Spectrum EnvironmentLight::apply(const PM::vec3& V)
	{
		if(!mMaterial || !mMaterial->emission())
			return Spectrum();

		ShaderClosure sc;
		sc.V = V;
		sc.Ng = V;
		sc.N = PM::pm_Negate(V);

		// Really do tangent?
		Projection::tangent_frame(sc.N, sc.Nx, sc.Ny);
		sc.UVW = PM::pm_ExtendTo3D(Projection::sphereUV(V));

		return mMaterial->emission()->eval(sc);
	}
}
