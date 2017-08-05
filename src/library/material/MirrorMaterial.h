#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB MirrorMaterial : public Material {
public:
	explicit MirrorMaterial(uint32 id);

	const std::shared_ptr<SpectralShaderOutput>& specularity() const;
	void setSpecularity(const std::shared_ptr<SpectralShaderOutput>& spec);

	const std::shared_ptr<SpectralShaderOutput>& ior() const;
	void setIOR(const std::shared_ptr<SpectralShaderOutput>& data);

	Spectrum eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd) override;

	std::string dumpInformation() const override;

private:
	std::shared_ptr<SpectralShaderOutput> mSpecularity;
	std::shared_ptr<SpectralShaderOutput> mIndex;
};
}
