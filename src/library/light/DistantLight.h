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

		inline void setMaterial(Material* m)
		{
			mMaterial = m;
		}

		inline Material* material() const
		{
			return mMaterial;
		}

		float pdf(const PM::vec3& L) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;
		Spectrum apply(const PM::vec3& L) override;

	private:
		PM::vec3 mDirection;
		Material* mMaterial;
	};
}