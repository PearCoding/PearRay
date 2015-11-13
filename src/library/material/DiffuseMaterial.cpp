#include "DiffuseMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/RandomRotationSphere.h"
#include "renderer/Renderer.h"

namespace PR
{
	DiffuseMaterial::DiffuseMaterial(const Spectrum& diffSpec) :
		Material(), mDiffSpectrum(diffSpec), mRoughness(1), mCanBeShaded(true)
	{
	}

	Spectrum DiffuseMaterial::spectrum() const
	{
		return mDiffSpectrum;
	}

	void DiffuseMaterial::setSpectrum(const Spectrum& diffSpec)
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

	void DiffuseMaterial::apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer)
	{
		Spectrum spec;

		float dot = PM::pm_Dot3D(in.direction(), point.normal());
		if (in.depth() < renderer->maxRayDepth() && mCanBeShaded)
		{
			float delta = mRoughness*PM_PI_F;

			// r = d - n*(d . n)
			PM::vec3 reflection = PM::pm_Normalize3D(PM::pm_Subtract(in.direction(),
				PM::pm_Scale(point.normal(), dot)));

			if (delta == 0) // Mirror
			{
				FacePoint collisionPoint;
				Ray ray(point.vertex(), reflection, in.depth()+1);
				renderer->shoot(ray, collisionPoint);
				spec = ray.spectrum();
			}
			else //
			{
				const int RAY_COUNT = renderer->maxRayBounceCount();

				float rph = std::acosf(
					PM::pm_MaxT<float>(-1, PM::pm_MinT<float>(1, PM::pm_GetZ(reflection))));
				float rrh = PM::pm_GetX(reflection) != 0 ? std::atan2f(PM::pm_GetY(reflection), PM::pm_GetX(reflection)) : 0;

				for (int i = 0; i < RAY_COUNT; ++i)
				{
					PM::vec3 norm2 = RandomRotationSphere::create(rph, rrh, -delta/2, delta/2, -delta, delta, renderer->random());
					
					FacePoint collisionPoint;
					Ray ray(point.vertex(), norm2, in.depth() + 1);
					renderer->shoot(ray, collisionPoint);
					
					float dot2 = std::fabsf(PM::pm_Dot3D(norm2, point.normal()));

					spec += dot2*ray.spectrum();

					//PR_DEBUG_ASSERT(!spec.hasNaN());
					//PR_DEBUG_ASSERT(!spec.hasInf());
				}

				spec /= (float)RAY_COUNT;
			}
		}
		
		spec = spec*mDiffSpectrum + mEmitSpectrum * fabsf(dot);//Really dot?

		//PR_DEBUG_ASSERT(!spec.hasNaN());
		//PR_DEBUG_ASSERT(!spec.hasInf());

		in.setSpectrum(spec);
	}
}