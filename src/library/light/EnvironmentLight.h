#pragma once

#include "IInfiniteLight.h"

namespace PR
{
	class Material;
	class PR_LIB EnvironmentLight : public IInfiniteLight
	{
	public:
		EnvironmentLight();
		virtual ~EnvironmentLight();

		inline void setMaterial(Material* m)
		{
			mMaterial = m;
		}

		inline Material* material() const
		{
			return mMaterial;
		}

		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;
		Spectrum apply(const PM::vec3& V) override;

	private:
		Material* mMaterial;
	};
}