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
		const float NdotV = PM::pm_Dot3D(point.normal(), V);
		const PM::vec3 N = NdotV < 0 ? PM::pm_Negate(point.normal()) : point.normal();

		Spectrum spec;
		if (mAlbedo && mRoughness)
		{
			if (alpha >= 1) // Lambert
			{
				spec = mAlbedo->eval(point.uv()) * Li * (PM_INV_PI_F);// Simple Lambert
			}
			else if (alpha >= PM_EPSILON)//Oren-Nayar
			{
				const float NdotL = std::abs(PM::pm_Dot3D(L, N));
				const float angleVN = acosf(NdotL);
				const float angleLN = acosf(NdotV);
				const float or_alpha = PM::pm_MaxT(angleLN, angleVN);
				const float or_beta = PM::pm_MinT(angleLN, angleVN);

				const float A = 1 - 0.5f * alpha / (alpha + 0.57f);
				const float B = 0.45f * alpha / (alpha + 0.09f);
				const float C = sinf(or_alpha) * tanf(or_beta);

				const float gamma = PM::pm_Dot3D(PM::pm_Subtract(V, PM::pm_Scale(N, NdotV)),
					PM::pm_Subtract(L, PM::pm_Scale(N, NdotL)));

				const float L1 = (A + B * C * PM::pm_MaxT(0.0f, gamma));

				spec = (PM_INV_PI_F * L1) * Li * mAlbedo->eval(point.uv());
			}
		}

		if (mSpecularity && mReflectivity)
		{
			const float refl = reflectivity(point);

			Spectrum specular = mSpecularity->eval(point.uv());

			float geometry = 1 / PM::pm_MaxT(0.00001f, PM::pm_MaxT(PM::pm_Dot3D(N, L), PM::pm_Dot3D(N, V)));// Neumann
			float ndf = BRDF::ndf_beckmann(H, N, alpha);
			float term = BRDF::fresnel_schlick_term(L, H);
			if (refl > PM_EPSILON && alpha < PM_EPSILON)
				term = BRDF::fresnel_schlick_term(L, N);

			for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
			{
				const float fres = fresnel(i/(float)Spectrum::SAMPLING_COUNT);
				const float tmp = NdotV < 0 ? (fres - 1) / (fres + 1) : (1 - fres) / (1 + fres);
				const float f0 = tmp*tmp;
				const float d = f0 + (1 - f0)*term;

				if (refl > PM_EPSILON && alpha > PM_EPSILON)
				{
					spec.setValue(i, spec.value(i) + (d * geometry * ndf * 0.25f) * Li.value(i) * specular.value(i));
				}
				else if (refl > PM_EPSILON)
				{
					spec.setValue(i, spec.value(i) + d * Li.value(i) * specular.value(i));
				}
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
		const float rough = roughness(point);
		const float alpha = rough * rough;
		const float refl = reflectivity(point);
		const float fres = fresnel(0);//TODO: Should use the average here.
		const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Add(L, V));

		const float tmp = (1 - fres) / (1 + fres);
		const float f0 = tmp*tmp;

		float ret;
		if (refl > PM_EPSILON && alpha > PM_EPSILON)
		{
			ret = BRDF::standard(f0, alpha, L, point.normal(), H, V);
		}
		else if (refl > PM_EPSILON)
		{
			ret = BRDF::fresnel_schlick(f0, L, point.normal());
		}

		return ret < PM_EPSILON ? 0.000001f : ret;// Shouldn't be zero
	}
}