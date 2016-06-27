#include "DiffuseMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "BRDF.h"
#include "math/Reflection.h"
#include "math/Projection.h"

namespace PR
{
	DiffuseMaterial::DiffuseMaterial() :
		Material(), mAlbedo(nullptr)
	{
	}

	Texture2D* DiffuseMaterial::albedo() const
	{
		return mAlbedo;
	}

	void DiffuseMaterial::setAlbedo(Texture2D* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	Spectrum DiffuseMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		return mAlbedo->eval(point.uv()) * (PM_INV_PI_F);
	}

	float DiffuseMaterial::pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		return PM_INV_PI_F*PM_INV_PI_F;
	}

	PM::vec3 DiffuseMaterial::sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf)
	{
		auto dir = Projection::align(point.normal(), Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd)));
		pdf = PM_INV_PI_F*PM_INV_PI_F;
		return dir;
	}
}