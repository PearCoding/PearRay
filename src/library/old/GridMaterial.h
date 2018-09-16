#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB GridMaterial : public Material {
public:
	explicit GridMaterial(uint32 id);

	void setFirstMaterial(const std::shared_ptr<Material>& mat);
	std::shared_ptr<Material> firstMaterial() const;

	void setSecondMaterial(const std::shared_ptr<Material>& mat);
	std::shared_ptr<Material> secondMaterial() const;

	void setGridCount(int i);
	int gridCount() const;

	void setTileUV(bool b);
	bool tileUV() const;

	void eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const override;
	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) const override;

	std::string dumpInformation() const override;

protected:
	void onFreeze(RenderContext* context) override;

private:
	ShaderClosure applyGrid(const ShaderClosure& point, int& u, int& v) const;

	std::shared_ptr<Material> mFirst;
	std::shared_ptr<Material> mSecond;

	int mGridCount;

	bool mTiledUV;
};
}
