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
		
		Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;

	private:
		SpectralShaderOutput* mAlbedo;
		ScalarShaderOutput* mRoughness;
	};
}