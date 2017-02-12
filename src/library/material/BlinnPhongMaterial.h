#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB BlinnPhongMaterial : public Material
	{
	public:
		BlinnPhongMaterial(uint32 id);

		SpectralShaderOutput* albedo() const;
		void setAlbedo(SpectralShaderOutput* diffSpec);

		ScalarShaderOutput* shininess() const;
		void setShininess(ScalarShaderOutput* data);

		//Normalized wavelength [0, 1] ~ 360 - 800
		SpectralShaderOutput* fresnelIndex() const;
		void setFresnelIndex(SpectralShaderOutput* data);

		Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;

		virtual std::string dumpInformation() const override;
	private:
		SpectralShaderOutput* mAlbedo;
		ScalarShaderOutput* mShininess;
		SpectralShaderOutput* mIndex;
	};
}
