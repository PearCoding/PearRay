#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB BRDFMaterial : public Material
	{
	public:
		BRDFMaterial();

		bool isLight() const;

		Spectrum albedo() const;
		void setAlbedo(const Spectrum& diffSpec);

		Spectrum specularity() const;
		void setSpecularity(const Spectrum& specSpec);

		float roughness() const;
		void setRoughness(float f);

		float reflectivity() const;
		void setReflectivity(float f);

		float fresnel() const;
		void setFresnel(float f);

		bool canBeShaded() const;
		void enableShading(bool b);

		void enableSelfShadow(bool b);
		bool canBeSelfShadowed() const;

		void enableCameraVisibility(bool b);
		bool isCameraVisible() const;

		void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer);
	private:
		Spectrum mAlbedoSpectrum;
		Spectrum mSpecularitySpectrum;
		float mRoughness;
		float mReflectivity;
		float mFresnel;

		bool mCanBeShaded;
		bool mSelfShadow;
		bool mCameraVisible;
	};
}