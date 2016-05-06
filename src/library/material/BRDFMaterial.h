#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB BRDFMaterial : public Material
	{
	public:
		BRDFMaterial();

		Spectrum albedo() const;
		void setAlbedo(const Spectrum& diffSpec);

		Spectrum specularity() const;
		void setSpecularity(const Spectrum& specSpec);

		float roughness() const override;
		void setRoughness(float f);

		float reflectivity() const;
		void setReflectivity(float f);

		float fresnel() const;
		void setFresnel(float f);

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li) override;

		float emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;
		float emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;

	private:
		Spectrum mAlbedoSpectrum;
		Spectrum mSpecularitySpectrum;

		float mRoughness;
		float mReflectivity;
		float mFresnel;
	};
}