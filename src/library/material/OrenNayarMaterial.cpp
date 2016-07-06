#include "OrenNayarMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "math/Projection.h"

namespace PR
{
	OrenNayarMaterial::OrenNayarMaterial() :
		Material(), mAlbedo(nullptr), mRoughness(nullptr)
	{
	}

	Texture2D* OrenNayarMaterial::albedo() const
	{
		return mAlbedo;
	}

	void OrenNayarMaterial::setAlbedo(Texture2D* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	Data2D* OrenNayarMaterial::roughness() const
	{
		return mRoughness;
	}

	void OrenNayarMaterial::setRoughness(Data2D* d)
	{
		mRoughness = d;
	}

	Spectrum OrenNayarMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		if (mAlbedo)
		{
			float val = PM_INV_PI_F;
			if (mRoughness)
			{
				float roughness = mRoughness->eval(point.uv());
				roughness *= roughness;// Square

				if (roughness > PM_EPSILON)// Oren Nayar
				{
					const float NdotV = std::abs(PM::pm_Dot3D(V, point.normal()));
					const float NdotL = std::abs(PM::pm_Dot3D(L, point.normal()));

					const float angleVN = std::acos(NdotV);
					const float angleLN = std::acos(NdotL);
					const float or_alpha = PM::pm_MaxT(angleLN, angleVN);
					const float or_beta = PM::pm_MinT(angleLN, angleVN);

					const float A = 1 - 0.5f * roughness / (roughness + 0.57f);
					const float B = 0.45f * roughness / (roughness + 0.09f);
					const float C = std::sin(or_alpha) * std::tan(or_beta);

					const float gamma = PM::pm_Dot3D(PM::pm_Add(V, PM::pm_Scale(point.normal(), NdotV)),
						PM::pm_Subtract(L, PM::pm_Scale(point.normal(), NdotL)));

					const float L1 = (A + B * C * PM::pm_MaxT(0.0f, gamma));

					val *= L1;
				}
			}// else lambert

			return mAlbedo->eval(point.uv()) * val;
		}
		else
			return Spectrum();
	}

	float OrenNayarMaterial::pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		return PM_INV_PI_F;
	}

	PM::vec3 OrenNayarMaterial::sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf)
	{
		auto dir = Projection::tangent_align(point.normal(), Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd)));
		pdf = PM_INV_PI_F;
		return dir;
	}
}