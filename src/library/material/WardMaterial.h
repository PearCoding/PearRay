#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB WardMaterial : public Material
	{
	public:
		WardMaterial(uint32 id);

		const std::shared_ptr<SpectralShaderOutput>& albedo() const;
		void setAlbedo(const std::shared_ptr<SpectralShaderOutput>& diffSpec);

		const std::shared_ptr<SpectralShaderOutput>& specularity() const;
		void setSpecularity(const std::shared_ptr<SpectralShaderOutput>& spec);

		const std::shared_ptr<ScalarShaderOutput>& roughnessX() const;
		void setRoughnessX(const std::shared_ptr<ScalarShaderOutput>& data);

		const std::shared_ptr<ScalarShaderOutput>& roughnessY() const;
		void setRoughnessY(const std::shared_ptr<ScalarShaderOutput>& data);

		const std::shared_ptr<ScalarShaderOutput>& reflectivity() const;
		void setReflectivity(const std::shared_ptr<ScalarShaderOutput>& data);

		Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;

		PM::vec3 samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, float& path_weight, uint32 path) override;
		uint32 samplePathCount() const override;

		virtual std::string dumpInformation() const override;
	private:
		PM::vec3 diffuse_path(const ShaderClosure& point, const PM::vec3& rnd, float& pdf);
		PM::vec3 specular_path(const ShaderClosure& point, const PM::vec3& rnd, float& pdf);

		std::shared_ptr<SpectralShaderOutput> mAlbedo;
		std::shared_ptr<SpectralShaderOutput> mSpecularity;
		std::shared_ptr<ScalarShaderOutput> mRoughnessX;
		std::shared_ptr<ScalarShaderOutput> mRoughnessY;
		std::shared_ptr<ScalarShaderOutput> mReflectivity;
	};
}
