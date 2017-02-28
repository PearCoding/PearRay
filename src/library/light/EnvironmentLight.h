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

	private:
		std::shared_ptr<Material> mMaterial;
	};
}
