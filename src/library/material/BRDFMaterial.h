#pragma once

#include "Material.h"
#include "PearMath.h"
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

		Spectrum emission() const;
		void setEmission(const Spectrum& spec);

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
		void applyOnRay(const PM::vec3& L, const PM::vec3& N, const PM::vec3& H, const PM::vec3& V, const Spectrum& E0,
			Spectrum& diff, Spectrum& spec);

		Spectrum mAlbedoSpectrum;
		Spectrum mSpecularitySpectrum;
		Spectrum mEmissionSpectrum;

		float mRoughness;
		float mReflectivity;
		float mFresnel;

		bool mCanBeShaded;
		bool mSelfShadow;
		bool mCameraVisible;
		bool mIsLight;
	};
}