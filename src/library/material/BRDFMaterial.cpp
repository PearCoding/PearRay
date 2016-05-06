#include "BRDFMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "BRDF.h"

namespace PR
{
	BRDFMaterial::BRDFMaterial() :
		Material(), mAlbedoSpectrum(), mSpecularitySpectrum(),
		mRoughness(1), mReflectivity(0), mFresnel(1)
	{
	}

	Spectrum BRDFMaterial::albedo() const
	{
		return mAlbedoSpectrum;
	}

	void BRDFMaterial::setAlbedo(const Spectrum& diffSpec)
	{
		mAlbedoSpectrum = diffSpec;
	}

	Spectrum BRDFMaterial::specularity() const
	{
		return mSpecularitySpectrum;
	}

	void BRDFMaterial::setSpecularity(const Spectrum& spec)
	{
		mSpecularitySpectrum = spec;
	}

	float BRDFMaterial::roughness() const
	{
		return mRoughness;
	}

	void BRDFMaterial::setRoughness(float f)
	{
		mRoughness = f;
	}

	float BRDFMaterial::reflectivity() const
	{
		return mReflectivity;
	}

	void BRDFMaterial::setReflectivity(float f)
	{
		mReflectivity = f;
	}

	float BRDFMaterial::fresnel() const
	{
		return mFresnel;
	}

	void BRDFMaterial::setFresnel(float f)
	{
		mFresnel = f;
	}

	Spectrum BRDFMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li)
	{
		const float alpha = mRoughness * mRoughness;
		const float NdotL = std::abs(PM::pm_Dot3D(L, point.normal()));
		const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Add(L, V));

		Spectrum spec;
		if (alpha >= 1) // Lambert
		{
			spec = mAlbedoSpectrum * Li * (PM_INV_PI_F);// Simple Lambert
		}
		else if (alpha >= PM_EPSILON)//Oren-Nayar
		{
			const float NdotV = PM::pm_Dot3D(point.normal(), V);
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

			spec = (PM_INV_PI_F * L1) * Li * mAlbedoSpectrum;
		}

		if (mReflectivity > PM_EPSILON && mRoughness > PM_EPSILON)
		{
			spec += BRDF::standard(mFresnel, alpha, L, point.normal(), H, V) * Li * mSpecularitySpectrum;
		}
		else if (mReflectivity > PM_EPSILON)
		{
			spec += BRDF::fresnel_schlick(mFresnel, L, H) * Li * mSpecularitySpectrum;
		}

		return spec;
	}

	float BRDFMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		dir = BRDF::reflect(point.normal(), V);
		return mReflectivity;
	}

	float BRDFMaterial::emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		return 0;
	}
}