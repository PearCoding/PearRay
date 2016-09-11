#include "DistantLight.h"
#include "shader/ShaderClosure.h"
#include "shader/ShaderOutput.h"
#include "spectral/Spectrum.h"
#include "material/Material.h"

namespace PR
{
	DistantLight::DistantLight() :
		mDirection(PM::pm_Set(0,0,1)), mMaterial(nullptr)
	{

	}

	DistantLight::~DistantLight()
	{
	}

	float DistantLight::pdf(const PM::vec3& L)
	{
		return std::numeric_limits<float>::infinity();
	}

	PM::vec3 DistantLight::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		pdf = std::numeric_limits<float>::infinity();
		return mDirection;
	}

	Spectrum DistantLight::apply(const PM::vec3& L)
	{
		if(!mMaterial || !mMaterial->emission())
			return Spectrum();

		const float d = std::abs(PM::pm_Dot3D(L, mDirection));

		ShaderClosure sc;
		sc.V = L;
		sc.Ng = mDirection;
		sc.N = mDirection;
		
		return mMaterial->emission()->eval(sc) * d;
	}
}