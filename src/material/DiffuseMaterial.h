#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class DiffuseMaterial
	{
	public:
		DiffuseMaterial(const Spectrum& diffSpec);

		Spectrum spectrum() const;
		void setSpectrum(const Spectrum& diffSpec);

		void apply(Ray& in, Entity* entity, const PM::vec3& point, const PM::vec3& normal, Renderer* renderer);
	private:
		Spectrum mDiffSpectrum;
	};
}