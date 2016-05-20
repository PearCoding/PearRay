#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"
#include "texture/Texture1D.h"

namespace PR
{
	class PR_LIB GlassMaterial : public Material
	{
	public:
		GlassMaterial();

		Texture2D* specularity() const;
		void setSpecularity(Texture2D* spec);

		float index(float lambda) const; //Normalized wavelength [0, 1] ~ 360 - 800
		Data1D* indexData() const;
		void setIndexData(Data1D* data);

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li) override;
		float pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;

		float emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;
		float emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;

		float roughness(const FacePoint& point) const override;
	private:
		Texture2D* mSpecularity;

		Data1D* mIndex;
	};
}