#include "DiffuseMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/RandomRotationSphere.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

namespace PR
{
	DiffuseMaterial::DiffuseMaterial() :
		Material(), mDiffSpectrum(), mRoughness(1), mCanBeShaded(true),
		mLight(false), mSelfShadow(true)
	{
	}

	void DiffuseMaterial::enableLight(bool b)
	{
		mLight = b;
	}

	bool DiffuseMaterial::isLight() const
	{
		return mLight;
	}

	Spectrum DiffuseMaterial::reflectance() const
	{
		return mDiffSpectrum;
	}

	void DiffuseMaterial::setReflectance(const Spectrum& diffSpec)
	{
		mDiffSpectrum = diffSpec;
	}

	Spectrum DiffuseMaterial::emission() const
	{
		return mEmitSpectrum;
	}

	void DiffuseMaterial::setEmission(const Spectrum& spec)
	{
		mEmitSpectrum = spec;
	}

	float DiffuseMaterial::roughness() const
	{
		return mRoughness;
	}

	void DiffuseMaterial::setRoughness(float f)
	{
		mRoughness = f;
	}

	bool DiffuseMaterial::canBeShaded() const
	{
		return mCanBeShaded;
	}

	void DiffuseMaterial::enableShading(bool b)
	{
		mCanBeShaded = b;
	}

	void DiffuseMaterial::enableSelfShadow(bool b)
	{
		mSelfShadow = b;
	}

	bool DiffuseMaterial::canBeSelfShadowed() const
	{
		return mSelfShadow;
	}

	void DiffuseMaterial::enableCameraVisibility(bool b)
	{
		mCameraVisible = b;
	}

	bool DiffuseMaterial::isCameraVisible() const
	{
		return mCameraVisible;
	}

	void DiffuseMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
		FacePoint collisionPoint;
		Spectrum spec;

		if (in.depth() < (in.maxDepth() == 0 ? renderer->maxRayDepth() : in.maxDepth()) &&
			mCanBeShaded)
		{
			float dot = PM::pm_Dot3D(in.direction(), point.normal());
			// r = d - 2*n*(d . n)
			PM::vec3 reflection = PM::pm_Normalize3D(PM::pm_Subtract(in.direction(),
				PM::pm_Scale(point.normal(), 2*dot)));

			if (mRoughness == 0) // Mirror
			{
				Ray ray(point.vertex(), reflection, in.depth()+1);
				renderer->shoot(ray, collisionPoint);
				spec = ray.spectrum();
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

						PM::vec3 dir = PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex()));

						Ray ray(point.vertex(), dir, in.depth()+1);// Bounce only once!
						ray.setMaxDepth(in.depth() + 1);

						RenderEntity* ent = renderer->shoot(ray, collisionPoint, mSelfShadow ? nullptr : entity);

						if (ent == light)// Full light!!
						{
							float dot2 = PM::pm_Dot3D(dir, point.normal());

							if (dot2 > 0)
							{
								spec += dot2 * ray.spectrum();
							}
						}

						lightSampleCounter++;
					}
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

					float dot2 = std::fabsf(PM::pm_Dot3D(norm2, point.normal()));

					spec += dot2 * ray.spectrum();

					//PR_DEBUG_ASSERT(!spec.hasNaN());
					//PR_DEBUG_ASSERT(!spec.hasInf());
					indirectSampleCounter++;
				}

				if (indirectSampleCounter + lightSampleCounter != 0)
				{
					spec *= PM_PI_F / (indirectSampleCounter + lightSampleCounter);
				}
			}
		}
		
		spec = spec * mDiffSpectrum;
		if (mCameraVisible || in.depth() > 0)
		{
			spec += mEmitSpectrum;
		}

		/*PR_DEBUG_ASSERT(!spec.hasNaN());
		PR_DEBUG_ASSERT(!spec.hasInf());*/

		in.setSpectrum(spec);
	}
}