#pragma once

#include "IInfiniteLight.h"

namespace PR
{
	class Material;
	class PR_LIB DistantLight : public IInfiniteLight
	{
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		
		DistantLight();
		virtual ~DistantLight();

		inline void setDirection(const Eigen::Vector3f& dir)
		{
			mDirection = dir;
		}

		inline Eigen::Vector3f direction() const
		{
			return mDirection;
		}

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

		void onFreeze() override;
	private:
		Eigen::Vector3f mDirection;
		std::shared_ptr<Material> mMaterial;

		Eigen::Vector3f mSampleDirection_Cache;
		Eigen::Vector3f mRight_Cache;
		Eigen::Vector3f mUp_Cache;
	};
}
