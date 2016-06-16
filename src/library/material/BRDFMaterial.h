#pragma once

#include "Material.h"
#include "texture/Texture1D.h"

namespace PR
{
	class PR_LIB BRDFMaterial : public Material
	{
	public:
		BRDFMaterial();

		Texture2D* albedo() const;
		void setAlbedo(Texture2D* diffSpec);

		Texture2D* specularity() const;
		void setSpecularity(Texture2D* specSpec);

		float roughness(const FacePoint& point) const;
		Data2D* roughnessData() const;
		void setRoughnessData(Data2D* data);

		float reflectivity(const FacePoint& point) const;
		Data2D* reflectivityData() const;
		void setReflectivityData(Data2D* data);

		float fresnel(float lambda) const; //Normalized wavelength [0, 1] ~ 360 - 800
		Data1D* fresnelData() const;
		void setFresnelData(Data1D* data);

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		float pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		PM::vec3 sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf) override;

	private:
		Texture2D* mAlbedo;
		Texture2D* mSpecularity;

		Data2D* mRoughness;
		Data2D* mReflectivity;
		Data1D* mFresnel;
	};
}