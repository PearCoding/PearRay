#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB WardMaterial : public Material
	{
	public:
		WardMaterial();

		SpectralShaderOutput* albedo() const;
		void setAlbedo(SpectralShaderOutput* diffSpec);

		SpectralShaderOutput* specularity() const;
		void setSpecularity(SpectralShaderOutput* spec);

		ScalarShaderOutput* roughnessX() const;
		void setRoughnessX(ScalarShaderOutput* data);

		ScalarShaderOutput* roughnessY() const;
		void setRoughnessY(ScalarShaderOutput* data);

		Spectrum apply(const SamplePoint& point, const PM::vec3& L) override;
		float pdf(const SamplePoint& point, const PM::vec3& L) override;
		PM::vec3 sample(const SamplePoint& point, const PM::vec3& rnd, float& pdf) override;

	private:
		SpectralShaderOutput* mAlbedo;
		SpectralShaderOutput* mSpecularity;
		ScalarShaderOutput* mRoughnessX;
		ScalarShaderOutput* mRoughnessY;
	};
}