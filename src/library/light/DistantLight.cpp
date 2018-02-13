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

IInfiniteLight::LightSample DistantLight::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session)
{
	PR_ASSERT(isFrozen(), "should be frozen.");

	IInfiniteLight::LightSample ls;
	ls.PDF_S = std::numeric_limits<float>::infinity();
	ls.L = mSampleDirection_Cache;
	return ls;
}

void DistantLight::apply(Spectrum& spec, const Eigen::Vector3f& V, const RenderSession& session)
{
	if (!mMaterial || !mMaterial->emission())
		return;

	PR_ASSERT(isFrozen(), "should be frozen.");

	const float d = std::max(0.0f, V.dot(mSampleDirection_Cache));

	ShaderClosure sc;
	sc.V  = V;
	sc.Ng = mDirection;
	sc.N  = mDirection;
	sc.Nx = mRight_Cache;
	sc.Ny = mUp_Cache;
	// UV is zero

	mMaterial->evalEmission(spec, sc, session);
	spec *= d;
}

void DistantLight::onFreeze()
{
	mSampleDirection_Cache = -mDirection;
	Projection::tangent_frame(mDirection, mRight_Cache, mUp_Cache);

	PR_LOGGER.logf(L_Info, M_Camera, "DistantLight Dir[%.3f,%.3f,%.3f]",
				   mDirection(0), mDirection(1), mDirection(2));
}
}
