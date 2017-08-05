#include "EnvironmentLight.h"
#include "material/Material.h"
#include "shader/ShaderClosure.h"
#include "shader/ShaderOutput.h"
#include "spectral/Spectrum.h"

#include "math/Projection.h"

namespace PR {
EnvironmentLight::EnvironmentLight()
	: mMaterial(nullptr)
{
}

EnvironmentLight::~EnvironmentLight()
{
}

IInfiniteLight::LightSample EnvironmentLight::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd)
{
	IInfiniteLight::LightSample ls;
	ls.L = Projection::tangent_align(point.N,
									 Projection::cos_hemi(rnd(0), rnd(1), ls.PDF));
	
	return ls;
}

Spectrum EnvironmentLight::apply(const Eigen::Vector3f& V)
{
	if (!mMaterial || !mMaterial->emission())
		return Spectrum();

	ShaderClosure sc;
	sc.V  = V;
	sc.Ng = V;
	sc.N  = -V;

	// Really do tangent?
	Projection::tangent_frame(sc.N, sc.Nx, sc.Ny);
	const Eigen::Vector2f uv = Projection::sphereUV(V);
	sc.UVW					 = Eigen::Vector3f(uv(0), uv(1), 0);

	return mMaterial->emission()->eval(sc);
}
}
