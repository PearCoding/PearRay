#pragma once

#include "Material.h"

namespace PR {
class PR_LIB OrenNayarMaterial : public Material {
public:
	explicit OrenNayarMaterial(uint32 id);

	const std::shared_ptr<SpectrumShaderOutput>& albedo() const;
	void setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec);

	const std::shared_ptr<ScalarShaderOutput>& roughness() const;
	void setRoughness(const std::shared_ptr<ScalarShaderOutput>& data);

	Spectrum eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd) override;

	std::string dumpInformation() const override;

private:
	std::shared_ptr<SpectrumShaderOutput> mAlbedo;
	std::shared_ptr<ScalarShaderOutput> mRoughness;
};
}
