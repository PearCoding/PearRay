#include "DistantLight.h"
#include "shader/ShaderClosure.h"
#include "shader/ShaderOutput.h"
#include "spectral/Spectrum.h"
#include "material/Material.h"

#include "math/Projection.h"

#include "Logger.h"

namespace PR
{
	DistantLight::DistantLight() :
		mDirection(PM::pm_Set(0,0,1)), mMaterial(nullptr)
	{

	}

	DistantLight::~DistantLight()
	{
	}

	PM::vec3 DistantLight::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		PR_ASSERT(isFrozen());

		pdf = std::numeric_limits<float>::infinity();
		return mSampleDirection_Cache;
		//return Projection::tangent_align(mSampleDirection_Cache, mRight_Cache, mUp_Cache,
		//	Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), 32, pdf)); 
	}

	Spectrum DistantLight::apply(const PM::vec3& V)
	{
		if(!mMaterial || !mMaterial->emission())
			return Spectrum();
		
		PR_ASSERT(isFrozen());

		const float d = PM::pm_MaxT(0.0f, PM::pm_Dot3D(V, mSampleDirection_Cache));

		ShaderClosure sc;
		sc.V = V;
		sc.Ng = mDirection;
		sc.N = mDirection;
		sc.Nx = mRight_Cache;
		sc.Ny = mUp_Cache;
		// UV is zero
		
		return mMaterial->emission()->eval(sc) * d;
	}

	void DistantLight::onFreeze()
	{
		mSampleDirection_Cache = PM::pm_Negate(mDirection); 
		Projection::tangent_frame(mDirection, mRight_Cache, mUp_Cache);

		PR_LOGGER.logf(L_Info, M_Camera,"DistantLight Dir[%.3f,%.3f,%.3f]",
			PM::pm_GetX(mDirection), PM::pm_GetY(mDirection), PM::pm_GetZ(mDirection));
	}
}