#pragma once

#include "Material.h"

namespace PR {
class PR_LIB BlinnPhongMaterial : public Material {
public:
	explicit BlinnPhongMaterial(uint32 id);

	const std::shared_ptr<SpectrumShaderOutput>& albedo() const;
	void setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec);

	const std::shared_ptr<ScalarShaderOutput>& shininess() const;
	void setShininess(const std::shared_ptr<ScalarShaderOutput>& data);

	//Normalized wavelength [0, 1] ~ 380 - 780
	const std::shared_ptr<SpectrumShaderOutput>& fresnelIndex() const;
	void setFresnelIndex(const std::shared_ptr<SpectrumShaderOutput>& data);

	Spectrum eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd) override;

	std::string dumpInformation() const override;

private:
	std::shared_ptr<SpectrumShaderOutput> mAlbedo;
	std::shared_ptr<ScalarShaderOutput> mShininess;
	std::shared_ptr<SpectrumShaderOutput> mIndex;
};
}
