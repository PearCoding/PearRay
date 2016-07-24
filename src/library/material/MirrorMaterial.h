#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB MirrorMaterial : public Material
	{
	public:
		MirrorMaterial();

		SpectralShaderOutput* specularity() const;
		void setSpecularity(SpectralShaderOutput* spec);

		SpectralShaderOutput* indexData() const;
		void setIndexData(SpectralShaderOutput* data);

		Spectrum apply(const SamplePoint& point, const PM::vec3& L) override;
		float pdf(const SamplePoint& point, const PM::vec3& L) override;
		PM::vec3 sample(const SamplePoint& point, const PM::vec3& rnd, float& pdf) override;
	private:
		SpectralShaderOutput* mSpecularity;
		SpectralShaderOutput* mIndex;
	};
}