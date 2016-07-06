#include "BlinnPhongMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "math/Projection.h"
#include "math/Fresnel.h"

namespace PR
{
	BlinnPhongMaterial::BlinnPhongMaterial() :
		Material(), mAlbedo(nullptr), mShininess(nullptr), mIndex(nullptr)
	{
	}

	Texture2D* BlinnPhongMaterial::albedo() const
	{
		return mAlbedo;
	}

	void BlinnPhongMaterial::setAlbedo(Texture2D* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	Data2D* BlinnPhongMaterial::shininess() const
	{
		return mShininess;
	}

	void BlinnPhongMaterial::setShininess(Data2D* d)
	{
		mShininess = d;
	}

	Data1D* BlinnPhongMaterial::fresnelIndex() const
	{
		return mIndex;
	}

	void BlinnPhongMaterial::setFresnelIndex(Data1D* data)
	{
		mIndex = data;
	}

	// TODO: Should be normalized better.
	Spectrum BlinnPhongMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		Spectrum albedo;
		if(mAlbedo)
		{
			albedo = mAlbedo->eval(point.uv()) * PM_INV_PI_F;
		}
		
		Spectrum spec;
		if (mIndex && mShininess)
		{
			const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Subtract(L, V));
			const float NdotH = std::abs(PM::pm_Dot3D(point.normal(), H));
			const float VdotH = std::abs(PM::pm_Dot3D(V, H));
			const float n = mShininess->eval(point.uv());

			for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
			{
				const float n2 = mIndex->eval(i / (float)Spectrum::SAMPLING_COUNT);
				const float f = Fresnel::dielectric(VdotH,
					!point.isInside() ? 1 : n2,
					!point.isInside() ? n2 : 1);

				spec.setValue(i, f);
			}

			spec *= std::pow(NdotH, n) * PM_INV_PI_F * (n + 4) / 8;
		}

		return albedo + spec;
	}

	float BlinnPhongMaterial::pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		if (mIndex)
		{
			const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Subtract(L, V));
			const float NdotH = std::abs(PM::pm_Dot3D(point.normal(), H));
			const float n = mShininess->eval(point.uv());
			return PM_INV_PI_F + std::pow(NdotH, n);
		}
		else
		{
			return PM_INV_PI_F;
		}
	}

	PM::vec3 BlinnPhongMaterial::sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf)
	{
		auto dir = Projection::tangent_align(point.normal(), Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd)));
		pdf = BlinnPhongMaterial::pdf(point, V, dir);
		return dir;
	}
}