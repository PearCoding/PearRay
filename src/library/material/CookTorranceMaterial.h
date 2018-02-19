#pragma once

#include "Material.h"
#include <vector>

namespace PR {
class PR_LIB CookTorranceMaterial : public Material {
public:
	enum FresnelMode {
		FM_Dielectric,
		FM_Conductor
	};

	enum DistributionMode {
		DM_Blinn,
		DM_Beckmann,
		DM_GGX
	};

	enum GeometryMode {
		GM_Implicit,
		GM_Neumann,
		GM_CookTorrance,
		GM_Kelemen
	};

	explicit CookTorranceMaterial(uint32 id);

	// Modes
	FresnelMode fresnelMode() const;
	void setFresnelMode(FresnelMode mode);

	DistributionMode distributionMode() const;
	void setDistributionMode(DistributionMode mode);

	GeometryMode geometryMode() const;
	void setGeometryMode(GeometryMode mode);

	// Diffuse
	std::shared_ptr<SpectrumShaderOutput> albedo() const;
	void setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec);

	std::shared_ptr<ScalarShaderOutput> diffuseRoughness() const;
	void setDiffuseRoughness(const std::shared_ptr<ScalarShaderOutput>& data);

	// Specular
	std::shared_ptr<SpectrumShaderOutput> specularity() const;
	void setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec);

	std::shared_ptr<ScalarShaderOutput> specularRoughnessX() const;
	void setSpecularRoughnessX(const std::shared_ptr<ScalarShaderOutput>& data);

	std::shared_ptr<ScalarShaderOutput> specularRoughnessY() const;
	void setSpecularRoughnessY(const std::shared_ptr<ScalarShaderOutput>& data);

	std::shared_ptr<SpectrumShaderOutput> ior() const;
	void setIOR(const std::shared_ptr<SpectrumShaderOutput>& data);

	std::shared_ptr<SpectrumShaderOutput> conductorAbsorption() const;
	void setConductorAbsorption(const std::shared_ptr<SpectrumShaderOutput>& data);

	std::shared_ptr<ScalarShaderOutput> reflectivity() const;
	void setReflectivity(const std::shared_ptr<ScalarShaderOutput>& data);

	void eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) override;

	MaterialSample samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path, const RenderSession& session) override;
	uint32 samplePathCount() const override;

	void setup(RenderContext* context) override;

	std::string dumpInformation() const override;

private:
	Eigen::Vector3f diffuse_path(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf);
	Eigen::Vector3f specular_path(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf);

	FresnelMode mFresnelMode;
	DistributionMode mDistributionMode;
	GeometryMode mGeometryMode;

	std::shared_ptr<SpectrumShaderOutput> mAlbedo;
	std::shared_ptr<ScalarShaderOutput> mDiffuseRoughness;

	std::shared_ptr<SpectrumShaderOutput> mSpecularity;
	std::shared_ptr<ScalarShaderOutput> mSpecRoughnessX;
	std::shared_ptr<ScalarShaderOutput> mSpecRoughnessY;
	std::shared_ptr<SpectrumShaderOutput> mIOR;
	std::shared_ptr<SpectrumShaderOutput> mConductorAbsorption;

	std::shared_ptr<ScalarShaderOutput> mReflectivity;

	std::vector<std::shared_ptr<struct CTM_ThreadData>> mThreadData;
};
}
