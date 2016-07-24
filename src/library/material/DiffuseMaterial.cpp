#include "DiffuseMaterial.h"
#include "ray/Ray.h"
#include "shader/SamplePoint.h"

#include "math/Projection.h"

namespace PR
{
	DiffuseMaterial::DiffuseMaterial() :
		Material(), mAlbedo(nullptr)
	{
	}

	SpectralShaderOutput* DiffuseMaterial::albedo() const
	{
		return mAlbedo;
	}

	void DiffuseMaterial::setAlbedo(SpectralShaderOutput* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	Spectrum DiffuseMaterial::apply(const SamplePoint& point, const PM::vec3& L)
	{
		if (mAlbedo)
			return mAlbedo->eval(point) * (PM_INV_PI_F);
		else
			return Spectrum();
	}

	float DiffuseMaterial::pdf(const SamplePoint& point, const PM::vec3& L)
	{
		return PM_INV_PI_F*PM_INV_PI_F;
	}

	PM::vec3 DiffuseMaterial::sample(const SamplePoint& point, const PM::vec3& rnd, float& pdf)
	{
		auto dir = Projection::tangent_align(point.N, Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd)));
		pdf = PM_INV_PI_F*PM_INV_PI_F;
		return dir;
	}
}