#include "DiffuseMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

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

	Spectrum DiffuseMaterial::apply(const ShaderClosure& point, const PM::vec3& L)
	{
		if (mAlbedo)
			return mAlbedo->eval(point) * (PM_INV_PI_F);
		else
			return Spectrum();
	}

	float DiffuseMaterial::pdf(const ShaderClosure& point, const PM::vec3& L)
	{
		return Projection::cos_hemi_pdf(point.N, L);
	}

	PM::vec3 DiffuseMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		auto dir = Projection::tangent_align(point.N,
			Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), pdf));
		return dir;
	}
}