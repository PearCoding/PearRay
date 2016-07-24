#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB OrenNayarMaterial : public Material
	{
	public:
		OrenNayarMaterial();

		SpectralShaderOutput* albedo() const;
		void setAlbedo(SpectralShaderOutput* diffSpec);

		ScalarShaderOutput* roughness() const;
		void setRoughness(ScalarShaderOutput* data);
		
		Spectrum apply(const SamplePoint& point, const PM::vec3& L) override;
		float pdf(const SamplePoint& point, const PM::vec3& L) override;
		PM::vec3 sample(const SamplePoint& point, const PM::vec3& rnd, float& pdf) override;

	private:
		SpectralShaderOutput* mAlbedo;
		ScalarShaderOutput* mRoughness;
	};
}