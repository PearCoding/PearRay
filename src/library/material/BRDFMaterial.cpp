#include "BRDFMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/RandomRotationSphere.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

#include "BRDF.h"

namespace PR
{
	BRDFMaterial::BRDFMaterial() :
		Material(), mAlbedoSpectrum(), mSpecularitySpectrum(), mRoughness(1), mReflectivity(0), mFresnel(1),
		mCanBeShaded(true), mSelfShadow(true), mCameraVisible(true)
	{
	}

	bool BRDFMaterial::isLight() const
	{
		return false;
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

	// Use a better normalization approach.
	constexpr float AlphaMax = 100000;
	void BRDFMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
		const float alpha = mRoughness * mRoughness;

		FacePoint collisionPoint;
		Spectrum diffuseTerm;
		Spectrum specTerm;
		Spectrum indirectTerm;// Ambient

		if (in.depth() < (in.maxDepth() == 0 ? renderer->maxRayDepth() : in.maxDepth()) &&
			mCanBeShaded)
		{
			const PM::vec3 N = PM::pm_SetW(point.normal(), 0);

			if ((1 - mReflectivity) <= std::numeric_limits<float>::epsilon() &&
				mRoughness <= std::numeric_limits<float>::epsilon()) // Mirror
			{
				float dot = PM::pm_Dot3D(in.direction(), point.normal());
				// r = d - 2*n*(d . n)
				PM::vec3 reflection = PM::pm_Normalize3D(PM::pm_Subtract(in.direction(),
					PM::pm_Scale(N, 2 * dot)));

				Ray ray(point.vertex(), reflection, in.depth() + 1);
				renderer->shoot(ray, collisionPoint);
				diffuseTerm = ray.spectrum();
			}
			else // Diffuse
			{ 
				// Direct Illumination
				uint32 lightSampleCounter = 0;
				for (RenderEntity* light : renderer->lights())
				{
					uint32 max = renderer->maxDirectRayCount();
					max = light->maxLightSamples() != 0 ? PM::pm_MinT(max, light->maxLightSamples()) : max;

					for (uint32 i = 0; i < max; ++i)
					{
						FacePoint p = light->getRandomFacePoint(renderer->random());

						const PM::vec3 L = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex())), 0);
						const float NdotL = PM::pm_MaxT(0.0f, PM::pm_Dot3D(L, N));

						if (NdotL > std::numeric_limits<float>::epsilon())
						{
							Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(L, 0.00001f)), L, in.depth() + 1);// Bounce only once!
							//ray.setFlags(0);
							ray.setMaxDepth(in.depth() + 1);

							RenderEntity* ent = renderer->shoot(ray, collisionPoint, mSelfShadow ? nullptr : entity);

							if (ent == light)// Full light!!
							{
								const PM::vec3 V = PM::pm_SetW(PM::pm_Negate(in.direction()), 0);
								const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Add(L, V));

								if (alpha <= std::numeric_limits<float>::epsilon()) // Lambert
								{
									diffuseTerm += PM_INV_PI_F * NdotL * ray.spectrum();// Simple Lambert
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

									diffuseTerm += L1 * ray.spectrum();
								}

								specTerm += mReflectivity *
									BRDF::standard(mFresnel, alpha, L, N, H, V) * ray.spectrum();

								lightSampleCounter++;
							}
						}
					}
				}

				if (lightSampleCounter != 0)
				{
					diffuseTerm /= lightSampleCounter;
					specTerm /= lightSampleCounter;
				}

				// Indirect Illumination
				// TODO: Add roughness factor
				//float rph = std::acosf(
				//	PM::pm_MaxT<float>(-1, PM::pm_MinT<float>(1, PM::pm_GetZ(reflection))));
				//float rrh = PM::pm_GetX(reflection) != 0 ? std::atan2f(PM::pm_GetY(reflection), PM::pm_GetX(reflection)) : 0;
				uint32 indirectSampleCounter = 0;
				for (int i = 0; i < renderer->maxIndirectRayCount(); ++i)
				{
					//PM::vec3 norm2 = RandomRotationSphere::create(rph, rrh, -delta/2, delta/2, -delta, delta, renderer->random());
					PM::vec3 norm2 = RandomRotationSphere::createFast(point.normal(), -1, 1, -1, 1, -1, 1, renderer->random());
					Ray ray(point.vertex(), norm2, in.depth() + 1);
					renderer->shoot(ray, collisionPoint);

					float dot = std::abs(PM::pm_Dot3D(norm2, point.normal()));
					indirectTerm += dot * ray.spectrum();
					indirectSampleCounter++;
				}

				if (indirectSampleCounter != 0)
				{
					indirectTerm /= indirectSampleCounter;
				}
			}
		}

		// Normalize spec term?
		in.setSpectrum((indirectTerm + diffuseTerm) * mAlbedoSpectrum + specTerm * mSpecularitySpectrum);
	}
}