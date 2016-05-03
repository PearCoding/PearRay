#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB GlassMaterial : public Material
	{
	public:
		GlassMaterial();

		Spectrum specularity() const;
		void setSpecularity(const Spectrum& spec);

		float index() const;
		void setIndex(float f);

		bool isLight() const;

		void enableCameraVisibility(bool b);
		bool isCameraVisible() const;

		void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer);

		bool shouldIgnore_Simple(const Ray& in, RenderEntity* entity) override;
	private:
		bool mCameraVisible;

		Spectrum mSpecularitySpectrum;

		float mIndex;
		float mFresnel;
	};
}