#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

#include <vector>

namespace PR {
class PR_LIB GlassMaterial : public Material {
public:
	explicit GlassMaterial(uint32 id);

	bool isThin() const;
	void setThin(bool b);

	std::shared_ptr<SpectrumShaderOutput> specularity() const;
	void setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec);

	std::shared_ptr<SpectrumShaderOutput> ior() const;
	void setIOR(const std::shared_ptr<SpectrumShaderOutput>& data);

	void eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) const override;

	MaterialSample samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path, const RenderSession& session) const override;
	uint32 samplePathCount() const override;

	std::string dumpInformation() const override;

protected:
	void onFreeze(RenderContext* context) override;

private:
	float get_weight(float ior, float NdotT, const ShaderClosure& point) const;
	void sample_reflection(MaterialSample& ms, const ShaderClosure& point, const RenderSession& session) const;
	void sample_refraction(MaterialSample& ms, float NdotT, float eta, const ShaderClosure& point, const RenderSession& session) const;
	std::shared_ptr<SpectrumShaderOutput> mSpecularity;
	std::shared_ptr<SpectrumShaderOutput> mIndex;
	bool mThin;
	uint32 mSpectrumSamples;
};
} // namespace PR
