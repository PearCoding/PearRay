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

	const std::shared_ptr<SpectrumShaderOutput>& specularity() const;
	void setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec);

	const std::shared_ptr<SpectrumShaderOutput>& ior() const;
	void setIOR(const std::shared_ptr<SpectrumShaderOutput>& data);

	void eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) override;

	MaterialSample samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path, const RenderSession& session) override;
	uint32 samplePathCount() const override;

	void setup(RenderContext* context) override;
	
	std::string dumpInformation() const override;

private:
	std::shared_ptr<SpectrumShaderOutput> mSpecularity;
	std::shared_ptr<SpectrumShaderOutput> mIndex;
	bool mThin;

	std::vector<struct GM_ThreadData> mThreadData;
};
}
