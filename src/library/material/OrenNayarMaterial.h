#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB OrenNayarMaterial : public Material
	{
	public:
		OrenNayarMaterial(uint32 id);

		const std::shared_ptr<SpectralShaderOutput>& albedo() const;
		void setAlbedo(const std::shared_ptr<SpectralShaderOutput>& diffSpec);

		const std::shared_ptr<ScalarShaderOutput>& roughness() const;
		void setRoughness(const std::shared_ptr<ScalarShaderOutput>& data);

		Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;

		virtual std::string dumpInformation() const override;
	private:
		std::shared_ptr<SpectralShaderOutput> mAlbedo;
		std::shared_ptr<ScalarShaderOutput> mRoughness;
	};
}
