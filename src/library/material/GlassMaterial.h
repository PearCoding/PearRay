#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB GlassMaterial : public Material {
public:
	explicit GlassMaterial(uint32 id);

	bool isThin() const;
	void setThin(bool b);

	const std::shared_ptr<SpectrumShaderOutput>& specularity() const;
	void setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec);

	const std::shared_ptr<SpectrumShaderOutput>& ior() const;
	void setIOR(const std::shared_ptr<SpectrumShaderOutput>& data);

	Spectrum eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd) override;

	MaterialSample samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path) override;
	uint32 samplePathCount() const override;

	std::string dumpInformation() const override;

private:
	std::shared_ptr<SpectrumShaderOutput> mSpecularity;
	std::shared_ptr<SpectrumShaderOutput> mIndex;
	bool mThin;
};
}
