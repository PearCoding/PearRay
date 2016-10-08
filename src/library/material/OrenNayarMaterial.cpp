#include "OrenNayarMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

#include "math/Projection.h"

#include "BRDF.h"

namespace PR
{
	OrenNayarMaterial::OrenNayarMaterial() :
		Material(), mAlbedo(nullptr), mRoughness(nullptr)
	{
	}

	SpectralShaderOutput* OrenNayarMaterial::albedo() const
	{
		return mAlbedo;
	}

	void OrenNayarMaterial::setAlbedo(SpectralShaderOutput* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	ScalarShaderOutput* OrenNayarMaterial::roughness() const
	{
		return mRoughness;
	}

	void OrenNayarMaterial::setRoughness(ScalarShaderOutput* d)
	{
		mRoughness = d;
	}

	Spectrum OrenNayarMaterial::eval(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		if (mAlbedo)
		{
			float val = PM_INV_PI_F;
			if (mRoughness)
			{
				float roughness = mRoughness->eval(point);
				roughness *= roughness;// Square

				if (roughness > PM_EPSILON)// Oren Nayar
					val = BRDF::orennayar(roughness, point.V, point.N, L, point.NdotV, NdotL);
			}// else lambert

			return mAlbedo->eval(point) * val;
		}
		else
			return Spectrum();
	}

	float OrenNayarMaterial::pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		return Projection::cos_hemi_pdf(NdotL);
	}

	PM::vec3 OrenNayarMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		auto dir = Projection::tangent_align(point.N,
			Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), pdf));
		return dir;
	}
}