#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB OrenNayarMaterial : public Material
	{
	public:
		OrenNayarMaterial(uint32 id);

		SpectralShaderOutput* albedo() const;
		void setAlbedo(SpectralShaderOutput* diffSpec);

		ScalarShaderOutput* roughness() const;
		void setRoughness(ScalarShaderOutput* data);
		
		Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;

		virtual std::string dumpInformation() const override;
	private:
		SpectralShaderOutput* mAlbedo;
		ScalarShaderOutput* mRoughness;
	};
}