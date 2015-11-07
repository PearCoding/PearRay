#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB DiffuseMaterial : public Material
	{
	public:
		DiffuseMaterial(const Spectrum& diffSpec);

		Spectrum spectrum() const;
		void setSpectrum(const Spectrum& diffSpec);

		Spectrum emission() const;
		void setEmission(const Spectrum& spec);

		float roughness() const;
		void setRoughness(float f);

		bool canBeShaded() const;
		void enableShading(bool b);

		void apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer);
	private:
		Spectrum mDiffSpectrum;
		Spectrum mEmitSpectrum;
		float mRoughness;
		bool mCanBeShaded;
	};
}