#pragma once

#include "IInfiniteLight.h"

namespace PR
{
	class Material;
	class PR_LIB DistantLight : public IInfiniteLight
	{
	public:
		DistantLight();
		virtual ~DistantLight();

		inline void setDirection(const PM::vec3& dir)
		{
			mDirection = dir;
		}

		inline PM::vec3 direction() const
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

		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;
		Spectrum apply(const PM::vec3& V) override;

		void onFreeze() override;
	private:
		PM::vec3 mDirection;
		std::shared_ptr<Material> mMaterial;

		PM::vec3 mSampleDirection_Cache;
		PM::vec3 mRight_Cache;
		PM::vec3 mUp_Cache;
	};
}
