#pragma once

#include "Material.h"

namespace PR {
class PR_LIB DiffuseMaterial : public Material {
public:
	explicit DiffuseMaterial(uint32 id);

	const std::shared_ptr<SpectrumShaderOutput>& albedo() const;
	void setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec);

	Spectrum eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd) override;

	std::string dumpInformation() const override;

private:
	std::shared_ptr<SpectrumShaderOutput> mAlbedo;
};
}
