#pragma once

#include "Material.h"
#include "texture/Texture1D.h"

namespace PR
{
	class PR_LIB DiffuseMaterial : public Material
	{
	public:
		DiffuseMaterial();

		Texture2D* albedo() const;
		void setAlbedo(Texture2D* diffSpec);
		
		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		float pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		PM::vec3 sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf) override;

	private:
		Texture2D* mAlbedo;
	};
}