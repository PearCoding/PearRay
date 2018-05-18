#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB MirrorMaterial : public Material {
public:
	explicit MirrorMaterial(uint32 id);

	std::shared_ptr<SpectrumShaderOutput> specularity() const;
	void setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec);

	std::shared_ptr<SpectrumShaderOutput> ior() const;
	void setIOR(const std::shared_ptr<SpectrumShaderOutput>& data);

	void eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) const override;

	std::string dumpInformation() const override;

protected:
	void onFreeze(RenderContext* context) override;

private:
	std::shared_ptr<SpectrumShaderOutput> mSpecularity;
	std::shared_ptr<SpectrumShaderOutput> mIndex;
};
}
