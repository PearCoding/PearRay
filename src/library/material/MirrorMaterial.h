#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB MirrorMaterial : public Material
	{
	public:
		MirrorMaterial(uint32 id);

		SpectralShaderOutput* specularity() const;
		void setSpecularity(SpectralShaderOutput* spec);

		SpectralShaderOutput* indexData() const;
		void setIndexData(SpectralShaderOutput* data);

		Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;
	private:
		SpectralShaderOutput* mSpecularity;
		SpectralShaderOutput* mIndex;
	};
}