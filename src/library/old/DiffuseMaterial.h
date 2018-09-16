#pragma once

#include "Material.h"

namespace PR {
class PR_LIB DiffuseMaterial : public Material {
public:
	explicit DiffuseMaterial(uint32 id);

	std::shared_ptr<SpectrumShaderOutput> albedo() const;
	void setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec);

	void eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) const override;

	std::string dumpInformation() const override;

protected:
	void onFreeze(RenderContext* context) override;

private:
	std::shared_ptr<SpectrumShaderOutput> mAlbedo;
};
}
