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

		if (in.depth() < renderer->maxRayDepth() && mCanBeShaded)// TODO
		{
			float delta = mRoughness*PM_PI_F;

			// r = d - n*(d . n)
			float dot =/* PM::pm_MinT<float>(0, */PM::pm_Dot3D(in.direction(), point.normal())/*)*/;
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
				for (int i = 0; i < RAY_COUNT; ++i)
				{
					PM::vec3 norm2 = RandomRotationSphere::create(reflection, -delta, delta, -delta, delta);
					
					FacePoint collisionPoint;
					Ray ray(point.vertex(), norm2, in.depth() + 1);
					renderer->shoot(ray, collisionPoint);
					
					float dot2 = std::fabsf(PM::pm_Dot3D(norm2, point.normal()));

					spec += dot2*ray.spectrum();
				}

				spec /= RAY_COUNT;
			}
		}
		
		spec = spec*mDiffSpectrum + mEmitSpectrum;

		in.setSpectrum(spec);
	}
}