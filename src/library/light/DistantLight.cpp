#include "DistantLight.h"
#include "material/Material.h"
#include "shader/ShaderClosure.h"
#include "shader/ShaderOutput.h"
#include "spectral/Spectrum.h"

#include "math/Projection.h"

#include "Logger.h"

namespace PR {
DistantLight::DistantLight()
	: mDirection(0, 0, 1)
	, mMaterial(nullptr)
{
}

DistantLight::~DistantLight()
{
}

Eigen::Vector3f DistantLight::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf)
{
	PR_ASSERT(isFrozen(), "should be frozen.");

	pdf = std::numeric_limits<float>::infinity();
	return mSampleDirection_Cache;
	//return Projection::tangent_align(mSampleDirection_Cache, mRight_Cache, mUp_Cache,
	//	Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), 32, pdf));
}

Spectrum DistantLight::apply(const Eigen::Vector3f& V)
{
	if (!mMaterial || !mMaterial->emission())
		return Spectrum();

	PR_ASSERT(isFrozen(), "should be frozen.");

	const float d = std::max(0.0f, V.dot(mSampleDirection_Cache));

	ShaderClosure sc;
	sc.V  = V;
	sc.Ng = mDirection;
	sc.N  = mDirection;
	sc.Nx = mRight_Cache;
	sc.Ny = mUp_Cache;
	// UV is zero

	return mMaterial->emission()->eval(sc) * d;
}

void DistantLight::onFreeze()
{
	mSampleDirection_Cache = -mDirection;
	Projection::tangent_frame(mDirection, mRight_Cache, mUp_Cache);

	PR_LOGGER.logf(L_Info, M_Camera, "DistantLight Dir[%.3f,%.3f,%.3f]",
				   mDirection(0), mDirection(1), mDirection(2));
}
}
