#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB GridMaterial : public Material {
public:
	explicit GridMaterial(uint32 id);

	void setFirstMaterial(const std::shared_ptr<Material>& mat);
	const std::shared_ptr<Material>& firstMaterial() const;

	void setSecondMaterial(const std::shared_ptr<Material>& mat);
	const std::shared_ptr<Material>& secondMaterial() const;

	void setGridCount(int i);
	int gridCount() const;

	void setTileUV(bool b);
	bool tileUV() const;

	Spectrum eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override;
	Eigen::Vector3f sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf) override;

	std::string dumpInformation() const override;

private:
	ShaderClosure applyGrid(const ShaderClosure& point, int& u, int& v) const;

	std::shared_ptr<Material> mFirst;
	std::shared_ptr<Material> mSecond;

	int mGridCount;

	bool mTiledUV;
};
}
