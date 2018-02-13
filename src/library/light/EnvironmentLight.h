#pragma once

#include "IInfiniteLight.h"

namespace PR {
class Material;
class PR_LIB EnvironmentLight : public IInfiniteLight {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EnvironmentLight();
	virtual ~EnvironmentLight();

	inline void setMaterial(const std::shared_ptr<Material>& m)
	{
		mMaterial = m;
	}

	inline const std::shared_ptr<Material>& material() const
	{
		return mMaterial;
	}

	LightSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) override;
	void apply(Spectrum& view, const Eigen::Vector3f& V, const RenderSession& session) override;

private:
	std::shared_ptr<Material> mMaterial;
};
}
