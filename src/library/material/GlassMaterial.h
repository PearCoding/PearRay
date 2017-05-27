#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB GlassMaterial : public Material {
public:
	explicit GlassMaterial(uint32 id);

	bool isThin() const;
	void setThin(bool b);

	const std::shared_ptr<SpectralShaderOutput>& specularity() const;
	void setSpecularity(const std::shared_ptr<SpectralShaderOutput>& spec);

	const std::shared_ptr<SpectralShaderOutput>& ior() const;
	void setIOR(const std::shared_ptr<SpectralShaderOutput>& data);

	Spectrum eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	Eigen::Vector3f sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf) override;

	Eigen::Vector3f samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf, float& path_weight, uint32 path) override;
	uint32 samplePathCount() const override;

	std::string dumpInformation() const override;

private:
	std::shared_ptr<SpectralShaderOutput> mSpecularity;
	std::shared_ptr<SpectralShaderOutput> mIndex;
	bool mThin;
};
}
