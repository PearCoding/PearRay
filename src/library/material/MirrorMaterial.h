#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"
#include "texture/Texture1D.h"

namespace PR
{
	class PR_LIB MirrorMaterial : public Material
	{
	public:
		MirrorMaterial();

		Texture2D* specularity() const;
		void setSpecularity(Texture2D* spec);

		float index(float lambda) const; //Normalized wavelength [0, 1] ~ 360 - 800
		Data1D* indexData() const;
		void setIndexData(Data1D* data);

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		float pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		PM::vec3 sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf) override;
	private:
		Texture2D* mSpecularity;

		Data1D* mIndex;
	};
}