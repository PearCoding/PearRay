#include "BRDFMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "sampler/Stratified2DSampler.h"
#include "sampler/Stratified3DSampler.h"
#include "sampler/Projection.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

#include "BRDF.h"

namespace PR
{
	BRDFMaterial::BRDFMaterial() :
		Material(), mAlbedoSpectrum(), mSpecularitySpectrum(), mEmissionSpectrum(),
		mRoughness(1), mReflectivity(0), mFresnel(1),
		mCanBeShaded(true), mSelfShadow(true), mCameraVisible(true), mIsLight(false)
	{
	}

	bool BRDFMaterial::isLight() const
	{
		return mIsLight;
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

	Spectrum BRDFMaterial::emission() const
	{
		return mEmissionSpectrum;
	}

	void BRDFMaterial::setEmission(const Spectrum& spec)
	{
		mEmissionSpectrum = spec;
		mIsLight = !mEmissionSpectrum.isOnlyZero();
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

	bool BRDFMaterial::canBeShaded() const
	{
		return mCanBeShaded;
	}

	void BRDFMaterial::enableShading(bool b)
	{
		mCanBeShaded = b;
	}

	void BRDFMaterial::enableSelfShadow(bool b)
	{
		mSelfShadow = b;
	}

	bool BRDFMaterial::canBeSelfShadowed() const
	{
		return mSelfShadow;
	}

	void BRDFMaterial::enableCameraVisibility(bool b)
	{
		mCameraVisible = b;
	}

	bool BRDFMaterial::isCameraVisible() const
	{
		return mCameraVisible;
	}

	constexpr float NormalOffset = 0.0001f;
	void BRDFMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
		const uint32 maxDepth = in.maxDepth() == 0 ? renderer->maxRayDepth() : in.maxDepth();
		if (in.depth() < maxDepth && (mCameraVisible || in.depth() > 0))
		{
			PR::Spectrum tmp;
			if (mCanBeShaded)
			{
				FacePoint collisionPoint;
				Spectrum diffuse;
				Spectrum spec;

				const PM::vec3 N = PM::pm_SetW(point.normal(), 0);
				uint32 sampleCounter = 0;

				// Direct Illumination
				for (RenderEntity* light : renderer->lights())
				{
					uint32 max = renderer->maxDirectRayCount();
					max = light->maxLightSamples() != 0 ? PM::pm_MinT(max, light->maxLightSamples()) : max;

					Stratified3DSampler sampler(renderer->maxDirectRayCount_3DSample(),
						renderer->maxDirectRayCount_3DSample(),
						renderer->maxDirectRayCount_3DSample());

					for (uint32 i = 0; i < max; ++i)
					{
						FacePoint p = light->getRandomFacePoint(sampler, renderer->random());

						const PM::vec3 L = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex())), 0);
						const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(L, N));

						if (NdotL > std::numeric_limits<float>::epsilon())
						{
							Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(L, NormalOffset)), L, in.depth() + 1);// Bounce only once!
							ray.setMaxDepth(in.depth() + 2);

							RenderEntity* ent = renderer->shoot(ray, collisionPoint, mSelfShadow ? nullptr : entity);

							if (ent == light)// Full light!!
							{
								const PM::vec3 V = PM::pm_SetW(PM::pm_Negate(in.direction()), 0);
								const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Add(L, V));
								applyOnRay(L, N, H, V, ray.spectrum(), diffuse, spec);
								sampleCounter++;
							}
						}
					}
				}

				// Simple stratified indirect solution
				Stratified2DSampler stratifiedSampler(renderer->maxIndirectRayCount_2DSample(), renderer->maxIndirectRayCount_2DSample());
				for (int i = 0; i < renderer->maxIndirectRayCount(); ++i)
				{
					const auto uv = stratifiedSampler.generate(renderer->random());

					PM::vec3 L = PM::pm_SetW(
						Projection::align(N, Projection::hemi(PM::pm_GetX(uv), PM::pm_GetY(uv))), 0);
					Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(L, NormalOffset)), L, in.depth() + 1);
					if (renderer->shoot(ray, collisionPoint))
					{
						const PM::vec3 V = PM::pm_SetW(in.direction(), 0);
						const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Add(L, V));
						applyOnRay(L, N, H, V, ray.spectrum(), diffuse, spec);
						sampleCounter++;
					}
				}

				if (sampleCounter != 0)
				{
					diffuse *= PM_PI_F / sampleCounter;
					spec *= PM_PI_F / sampleCounter;
				}

				tmp = diffuse * mAlbedoSpectrum + spec * mSpecularitySpectrum;
			}

			if (mIsLight)
			{
				tmp += PM::pm_MaxT(0.0f, -PM::pm_Dot3D(in.direction(), point.normal())) * mEmissionSpectrum;
			}

			in.setSpectrum(tmp);
		}
	}

	void BRDFMaterial::applyOnRay(const PM::vec3& L, const PM::vec3& N, const PM::vec3& H, const PM::vec3& V,
		const Spectrum& E0, Spectrum& diff, Spectrum& spec)
	{
		const float alpha = mRoughness * mRoughness;
		const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(L, N));

		if (alpha <= std::numeric_limits<float>::epsilon()) // Lambert
		{
			diff += PM_INV_PI_F * NdotL * E0;// Simple Lambert
		}
		else//Oren-Nayar
		{
			const float NdotV = PM::pm_Dot3D(N, V);
			const float angleVN = acosf(NdotL);
			const float angleLN = acosf(NdotV);
			const float or_alpha = PM::pm_MaxT(angleLN, angleVN);
			const float or_beta = PM::pm_MinT(angleLN, angleVN);

			const float A = 1 - 0.5f * alpha / (alpha + 0.57f);
			const float B = 0.45f * alpha / (alpha + 0.09f);
			const float C = sinf(or_alpha) * tanf(or_beta);

			const float gamma = PM::pm_Dot3D(PM::pm_Subtract(V, PM::pm_Scale(N, NdotV)),
				PM::pm_Subtract(L, PM::pm_Scale(N, NdotL)));

			const float L1 = NdotL * (A + B * C * PM::pm_MaxT(0.0f, gamma));

			diff += L1 * E0;
		}

		if (mReflectivity > std::numeric_limits<float>::epsilon())
		{
			spec += mReflectivity *
				BRDF::standard(mFresnel, alpha, L, N, H, V) * E0;
		}
	}
}