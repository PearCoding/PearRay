#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class DiffuseMaterial : public Material
	{
	public:
		DiffuseMaterial(const Spectrum& diffSpec);

		Spectrum spectrum() const;
		void setSpectrum(const Spectrum& diffSpec);

		Spectrum emission() const;
		void setEmission(const Spectrum& spec);

		float glossyness() const;
		void setGlossyness(float f);

		void apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer);
	private:
		Spectrum mDiffSpectrum;
		Spectrum mEmitSpectrum;
		float mGlossyness;
	};
}