#include "DiffuseMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

namespace PR
{
	DiffuseMaterial::DiffuseMaterial(const Spectrum& diffSpec) :
		Material(), mDiffSpectrum(diffSpec)
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

	void DiffuseMaterial::apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer)
	{
		// Could save two copy operations, but should be done in another anyway.
		Spectrum spec = in.spectrum();

		float aff = std::fabsf(PM::pm_Dot3D(in.direction(), point.normal()));

		for (size_t i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			spec.setValue(i, spec.value(i)*mDiffSpectrum.value(i)*aff);
		}

		in.setSpectrum(spec);
	}
}