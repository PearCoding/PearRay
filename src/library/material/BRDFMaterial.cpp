#include "BRDFMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "BRDF.h"

namespace PR
{
	BRDFMaterial::BRDFMaterial() :
		Material(), mAlbedo(nullptr), mSpecularity(nullptr),
		mRoughness(nullptr), mReflectivity(nullptr), mFresnel(nullptr)
	{
	}

	Texture2D* BRDFMaterial::albedo() const
	{
		return mAlbedo;
	}

	void BRDFMaterial::setAlbedo(Texture2D* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	Texture2D* BRDFMaterial::specularity() const
	{
		return mSpecularity;
	}

	void BRDFMaterial::setSpecularity(Texture2D* spec)
	{
		mSpecularity = spec;
	}

	float BRDFMaterial::roughness(const FacePoint& point) const
	{
		if (mRoughness)
			return mRoughness->eval(point.uv());
		else
			return 0;
	}

	Data2D* BRDFMaterial::roughnessData() const
	{
		return mRoughness;
	}

	void BRDFMaterial::setRoughnessData(Data2D* data)
	{
		mRoughness = data;
	}

	float BRDFMaterial::reflectivity(const FacePoint& point) const
	{
		if (mReflectivity)
			return mReflectivity->eval(point.uv());
		else
			return 0;
	}

	Data2D* BRDFMaterial::reflectivityData() const
	{
		return mReflectivity;
	}

	void BRDFMaterial::setReflectivityData(Data2D* data)
	{
		mReflectivity = data;
	}

	float BRDFMaterial::fresnel(float lambda) const
	{
		if (mFresnel)
			return mFresnel->eval(lambda);
		else
			return 0;
	}

	Data1D* BRDFMaterial::fresnelData() const
	{
		return mFresnel;
	}

	void BRDFMaterial::setFresnelData(Data1D* data)
	{
		mFresnel = data;
	}

	Spectrum BRDFMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li)
	{
		const float rough = roughness(point);
		const float alpha = rough * rough;
		const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Add(L, V));

		Spectrum spec;
		if (mAlbedo && mRoughness)
		{
			if (alpha >= 1) // Lambert
			{
				spec = mAlbedo->eval(point.uv()) * Li * (PM_INV_PI_F);// Simple Lambert
			}
			else if (alpha >= PM_EPSILON)//Oren-Nayar
			{
				const float NdotV = std::abs(PM::pm_Dot3D(point.normal(), V));
				const float NdotL = std::abs(PM::pm_Dot3D(L, point.normal()));
				const float angleVN = acosf(NdotL);
				const float angleLN = acosf(NdotV);
				const float or_alpha = PM::pm_MaxT(angleLN, angleVN);
				const float or_beta = PM::pm_MinT(angleLN, angleVN);

				const float A = 1 - 0.5f * alpha / (alpha + 0.57f);
				const float B = 0.45f * alpha / (alpha + 0.09f);
				const float C = sinf(or_alpha) * tanf(or_beta);

				const float gamma = PM::pm_Dot3D(PM::pm_Subtract(V, PM::pm_Scale(point.normal(), NdotV)),
					PM::pm_Subtract(L, PM::pm_Scale(point.normal(), NdotL)));

				const float L1 = (A + B * C * PM::pm_MaxT(0.0f, gamma));

				spec = (PM_INV_PI_F * L1) * Li * mAlbedo->eval(point.uv());
			}
		}

		if (mSpecularity && mReflectivity)
		{
			const float refl = reflectivity(point);
			const float fres = fresnel(0);//TODO
			if (refl > PM_EPSILON && alpha > PM_EPSILON)
			{
				spec += BRDF::standard(fres, alpha, L, point.normal(), H, V) * Li * mSpecularity->eval(point.uv());
			}
			else if (refl > PM_EPSILON)
			{
				const float d = 1 - BRDF::fresnel_schlick(fres, L, point.normal());
				spec +=  d * Li * mSpecularity->eval(point.uv());
			}
		}

		return spec;
	}

	float BRDFMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		dir = BRDF::reflect(point.normal(), V);
		return reflectivity(point);
	}

	float BRDFMaterial::emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		return 0;
	}

	float BRDFMaterial::pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		// TODO:
		return 1
			;
	}
}