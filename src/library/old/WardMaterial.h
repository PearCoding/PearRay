#pragma once

#include "Material.h"
#include <vector>

namespace PR {
class PR_LIB WardMaterial : public Material {
public:
	explicit WardMaterial(uint32 id);

	std::shared_ptr<SpectrumShaderOutput> albedo() const;
	void setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec);

	std::shared_ptr<SpectrumShaderOutput> specularity() const;
	void setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec);

	std::shared_ptr<ScalarShaderOutput> roughnessX() const;
	void setRoughnessX(const std::shared_ptr<ScalarShaderOutput>& data);

	std::shared_ptr<ScalarShaderOutput> roughnessY() const;
	void setRoughnessY(const std::shared_ptr<ScalarShaderOutput>& data);

	std::shared_ptr<ScalarShaderOutput> reflectivity() const;
	void setReflectivity(const std::shared_ptr<ScalarShaderOutput>& data);

	void eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) const override;

	MaterialSample samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path, const RenderSession& session) const override;
	uint32 samplePathCount() const override;

	std::string dumpInformation() const override;

protected:
	void onFreeze(RenderContext* context) override;

private:
	Eigen::Vector3f diffuse_path(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf) const;
	Eigen::Vector3f specular_path(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf) const;

	std::shared_ptr<SpectrumShaderOutput> mAlbedo;
	std::shared_ptr<SpectrumShaderOutput> mSpecularity;
	std::shared_ptr<ScalarShaderOutput> mRoughnessX;
	std::shared_ptr<ScalarShaderOutput> mRoughnessY;
	std::shared_ptr<ScalarShaderOutput> mReflectivity;

	std::vector<std::shared_ptr<struct WM_ThreadData>> mThreadData;
};
}
