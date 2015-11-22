#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB DiffuseMaterial : public Material
	{
	public:
		DiffuseMaterial();

		void enableLight(bool b);
		bool isLight() const;

		Spectrum reflectance() const;
		void setReflectance(const Spectrum& diffSpec);

		Spectrum emission() const;
		void setEmission(const Spectrum& spec);

		float roughness() const;
		void setRoughness(float f);

		bool canBeShaded() const;
		void enableShading(bool b);

		void enableSelfShadow(bool b);
		bool canBeSelfShadowed() const;

		void enableCameraVisibility(bool b);
		bool isCameraVisible() const;

		void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer);
	private:
		Spectrum mDiffSpectrum;
		Spectrum mEmitSpectrum;
		float mRoughness;
		bool mCanBeShaded;
		bool mLight;
		bool mSelfShadow;
		bool mCameraVisible;
	};
}