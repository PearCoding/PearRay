#pragma once

#include "Material.h"
#include "texture/Texture2D.h"
#include "texture/Texture1D.h"

namespace PR
{
	class PR_LIB BlinnPhongMaterial : public Material
	{
	public:
		BlinnPhongMaterial();

		Texture2D* albedo() const;
		void setAlbedo(Texture2D* diffSpec);

		Data2D* shininess() const;
		void setShininess(Data2D* data);

		//Normalized wavelength [0, 1] ~ 360 - 800
		Data1D* fresnelIndex() const;
		void setFresnelIndex(Data1D* data);

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		float pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		PM::vec3 sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf) override;

	private:
		Texture2D* mAlbedo;
		Data2D* mShininess;
		Data1D* mIndex;
	};
}