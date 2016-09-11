#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB GlassMaterial : public Material
	{
	public:
		GlassMaterial();

		SpectralShaderOutput* specularity() const;
		void setSpecularity(SpectralShaderOutput* spec);

		SpectralShaderOutput* indexData() const;
		void setIndexData(SpectralShaderOutput* data);

		Spectrum apply(const ShaderClosure& point, const PM::vec3& L) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;
	private:
		SpectralShaderOutput* mSpecularity;
		SpectralShaderOutput* mIndex;
	};
}