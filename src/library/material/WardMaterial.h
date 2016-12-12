#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB WardMaterial : public Material
	{
	public:
		WardMaterial(uint32 id);

		SpectralShaderOutput* albedo() const;
		void setAlbedo(SpectralShaderOutput* diffSpec);

		SpectralShaderOutput* specularity() const;
		void setSpecularity(SpectralShaderOutput* spec);

		ScalarShaderOutput* roughnessX() const;
		void setRoughnessX(ScalarShaderOutput* data);

		ScalarShaderOutput* roughnessY() const;
		void setRoughnessY(ScalarShaderOutput* data);

		ScalarShaderOutput* reflectivity() const;
		void setReflectivity(ScalarShaderOutput* data);

		Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;

		PM::vec3 samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, Spectrum& path_weight, uint32 path) override;
		uint32 samplePathCount() const override;
		
		virtual std::string dumpInformation() const override;
	private:
		PM::vec3 diffuse_path(const ShaderClosure& point, const PM::vec3& rnd, float& pdf);
		PM::vec3 specular_path(const ShaderClosure& point, const PM::vec3& rnd, float& pdf);

		SpectralShaderOutput* mAlbedo;
		SpectralShaderOutput* mSpecularity;
		ScalarShaderOutput* mRoughnessX;
		ScalarShaderOutput* mRoughnessY;
		ScalarShaderOutput* mReflectivity;
	};
}