#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB BlinnPhongMaterial : public Material
	{
	public:
		BlinnPhongMaterial();

		SpectralShaderOutput* albedo() const;
		void setAlbedo(SpectralShaderOutput* diffSpec);

		ScalarShaderOutput* shininess() const;
		void setShininess(ScalarShaderOutput* data);

		//Normalized wavelength [0, 1] ~ 360 - 800
		SpectralShaderOutput* fresnelIndex() const;
		void setFresnelIndex(SpectralShaderOutput* data);

		Spectrum apply(const SamplePoint& point, const PM::vec3& L) override;
		float pdf(const SamplePoint& point, const PM::vec3& L) override;
		PM::vec3 sample(const SamplePoint& point, const PM::vec3& rnd, float& pdf) override;

	private:
		SpectralShaderOutput* mAlbedo;
		ScalarShaderOutput* mShininess;
		SpectralShaderOutput* mIndex;
	};
}