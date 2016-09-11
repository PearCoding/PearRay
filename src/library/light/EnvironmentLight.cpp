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

	float EnvironmentLight::pdf(const PM::vec3& L)
	{
		return Projection::sphere_pdf();
	}

	PM::vec3 EnvironmentLight::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		return Projection::sphere(PM::pm_GetX(rnd), PM::pm_GetY(rnd), pdf);
	}

	Spectrum EnvironmentLight::apply(const PM::vec3& L)
	{
		if(!mMaterial || !mMaterial->emission())
			return Spectrum();

		ShaderClosure sc;
		sc.V = L;
		sc.Ng = PM::pm_Negate(L);
		sc.N = sc.Ng;
		
		// Really do tangent?
		Projection::tangent_frame(sc.N, sc.Nx, sc.Ny);
		sc.UV = Projection::sphereUV(L);
		
		return mMaterial->emission()->eval(sc);
	}
}