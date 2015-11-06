#include "DiffuseMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/RandomRotationSphere.h"
#include "renderer/Renderer.h"

namespace PR
{
	DiffuseMaterial::DiffuseMaterial(const Spectrum& diffSpec) :
		Material(), mDiffSpectrum(diffSpec), mGlossyness(1)
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

	float DiffuseMaterial::glossyness() const
	{
		return mGlossyness;
	}

	void DiffuseMaterial::setGlossyness(float f)
	{
		mGlossyness = f;
	}

	void DiffuseMaterial::apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer)
	{
		Spectrum spec;

		if (in.depth() < 2)// TODO
		{
			float delta = (1 - mGlossyness)*PM_PI_F;
			if (delta == 0) // Mirror
			{
				// r = d - 2*n*(d . n)
				PM::vec3 reflection = PM::pm_Subtract(in.direction(),
					PM::pm_Scale(point.normal(), 2 * PM::pm_Dot3D(in.direction(), point.normal())));

				FacePoint collisionPoint;
				Ray ray(point.vertex(), reflection, in.depth()+1);
				renderer->shoot(ray, collisionPoint);
				spec = ray.spectrum();
			}
			else //
			{
				const int RAY_COUNT = 50;
				for (int i = 0; i < RAY_COUNT; ++i)
				{
					PM::vec3 norm2 = RandomRotationSphere::create(point.normal(), -delta, delta, -delta, delta);

					// r = d - 2*n*(d . n)
					PM::vec3 reflection = PM::pm_Subtract(in.direction(),
						PM::pm_Scale(point.normal(), 2 * PM::pm_Dot3D(in.direction(), point.normal())));

					FacePoint collisionPoint;
					Ray ray(point.vertex(), reflection, in.depth() + 1);
					renderer->shoot(ray, collisionPoint);
					
					Spectrum tmp = ray.spectrum();
					for (int j = 0; j < Spectrum::SAMPLING_COUNT; ++j)
					{
						spec.setValue(j, spec.value(j) + tmp.value(j));
					}
				}

				for (int j = 0; j < Spectrum::SAMPLING_COUNT; ++j)
				{
					spec.setValue(j, spec.value(j) / RAY_COUNT);
				}
			}
		}
		
		for (size_t i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			spec.setValue(i, spec.value(i)*mDiffSpectrum.value(i) + mEmitSpectrum.value(i));
		}

		in.setSpectrum(spec);
	}
}