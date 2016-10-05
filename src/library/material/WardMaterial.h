#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB WardMaterial : public Material
	{
	public:
		WardMaterial();

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

	private:
		SpectralShaderOutput* mAlbedo;
		SpectralShaderOutput* mSpecularity;
		ScalarShaderOutput* mRoughnessX;
		ScalarShaderOutput* mRoughnessY;
		ScalarShaderOutput* mReflectivity;
	};
}