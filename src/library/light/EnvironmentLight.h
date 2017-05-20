#pragma once

#include "IInfiniteLight.h"

namespace PR
{
	class Material;
	class PR_LIB EnvironmentLight : public IInfiniteLight
	{
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

		Eigen::Vector3f sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf) override;
		Spectrum apply(const Eigen::Vector3f& V) override;

	private:
		std::shared_ptr<Material> mMaterial;
	};
}
