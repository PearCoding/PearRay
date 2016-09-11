#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB DiffuseMaterial : public Material
	{
	public:
		DiffuseMaterial();

		SpectralShaderOutput* albedo() const;
		void setAlbedo(SpectralShaderOutput* diffSpec);
		
		Spectrum apply(const ShaderClosure& point, const PM::vec3& L) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;

	private:
		SpectralShaderOutput* mAlbedo;
	};
}