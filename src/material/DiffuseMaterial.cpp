#include "DiffuseMaterial.h"
#include "ray/Ray.h"

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

	void DiffuseMaterial::apply(Ray& in, Entity* entity, const PM::vec3& point, const PM::vec3& normal, Renderer* renderer)
	{
		// Could save two copy operations, but should be done in another anyway.
		Spectrum spec = in.spectrum();

		for (size_t i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			spec.setValue(i, spec.value(i)*mDiffSpectrum.value(i));
		}

		in.setSpectrum(spec);
	}
}