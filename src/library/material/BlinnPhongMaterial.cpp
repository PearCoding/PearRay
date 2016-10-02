#include "BlinnPhongMaterial.h"
#include "ray/Ray.h"

#include "math/Projection.h"
#include "math/Fresnel.h"
#include "shader/ShaderClosure.h"

namespace PR
{
	BlinnPhongMaterial::BlinnPhongMaterial() :
		Material(), mAlbedo(nullptr), mShininess(nullptr), mIndex(nullptr)
	{
	}

	SpectralShaderOutput* BlinnPhongMaterial::albedo() const
	{
		return mAlbedo;
	}

	void BlinnPhongMaterial::setAlbedo(SpectralShaderOutput* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	ScalarShaderOutput* BlinnPhongMaterial::shininess() const
	{
		return mShininess;
	}

	void BlinnPhongMaterial::setShininess(ScalarShaderOutput* d)
	{
		mShininess = d;
	}

	SpectralShaderOutput* BlinnPhongMaterial::fresnelIndex() const
	{
		return mIndex;
	}

	void BlinnPhongMaterial::setFresnelIndex(SpectralShaderOutput* data)
	{
		mIndex = data;
	}

	// TODO: Should be normalized better.
	Spectrum BlinnPhongMaterial::eval(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		Spectrum albedo;
		if(mAlbedo)
			albedo = mAlbedo->eval(point) * PM_INV_PI_F;
		
		Spectrum spec;
		if (mIndex && mShininess)
		{
			const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Subtract(L, point.V));
			const float NdotH = std::abs(PM::pm_Dot3D(point.N, H));
			const float VdotH = std::abs(PM::pm_Dot3D(point.V, H));
			const float n = mShininess->eval(point);
			Spectrum index = mIndex->eval(point);

			for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
			{
				const float n2 = index.value(i);
				const float f = Fresnel::dielectric(VdotH,
					!(point.Flags & SCF_Inside) ? 1 : n2,
					!(point.Flags & SCF_Inside) ? n2 : 1);

				spec.setValue(i, f);
			}

			spec *= std::pow(NdotH, n) * PM_INV_PI_F * (n + 4) / 8;
		}

		return albedo + spec;
	}

	float BlinnPhongMaterial::pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		if (mIndex)
		{
			const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Subtract(L, point.V));
			const float NdotH = std::abs(PM::pm_Dot3D(point.N, H));
			const float n = mShininess->eval(point);
			return PM_INV_PI_F + std::pow(NdotH, n);
		}
		else
		{
			return PM_INV_PI_F;
		}
	}

	PM::vec3 BlinnPhongMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		auto dir = Projection::tangent_align(point.N,
			Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), pdf));
		pdf += BlinnPhongMaterial::pdf(point, dir, 0);
		return dir;
	}
}