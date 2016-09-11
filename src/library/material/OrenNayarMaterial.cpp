#include "OrenNayarMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

#include "math/Projection.h"

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

	Spectrum OrenNayarMaterial::apply(const ShaderClosure& point, const PM::vec3& L)
	{
		if (mAlbedo)
		{
			float val = PM_INV_PI_F;
			if (mRoughness)
			{
				float roughness = mRoughness->eval(point);
				roughness *= roughness;// Square

				if (roughness > PM_EPSILON)// Oren Nayar
				{
					const float NdotL = std::abs(PM::pm_Dot3D(L, point.N));

					const float angleVN = std::acos(point.NdotV);
					const float angleLN = std::acos(NdotL);
					const float or_alpha = PM::pm_MaxT(angleLN, angleVN);
					const float or_beta = PM::pm_MinT(angleLN, angleVN);

					const float A = 1 - 0.5f * roughness / (roughness + 0.57f);
					const float B = 0.45f * roughness / (roughness + 0.09f);
					const float C = std::sin(or_alpha) * std::tan(or_beta);

					const float gamma = PM::pm_Dot3D(PM::pm_Add(point.V, PM::pm_Scale(point.N, point.NdotV)),
						PM::pm_Subtract(L, PM::pm_Scale(point.N, NdotL)));

					const float L1 = (A + B * C * PM::pm_MaxT(0.0f, gamma));

					val *= L1;
				}
			}// else lambert

			return mAlbedo->eval(point) * val;
		}
		else
			return Spectrum();
	}

	float OrenNayarMaterial::pdf(const ShaderClosure& point, const PM::vec3& L)
	{
		return Projection::cos_hemi_pdf(point.N, L);
	}

	PM::vec3 OrenNayarMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		auto dir = Projection::tangent_align(point.N,
			Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), pdf));
		return dir;
	}
}