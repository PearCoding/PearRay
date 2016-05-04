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

		void apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li,
			Spectrum& diff, Spectrum& spec) override;

		float emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;
		float emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;

		float roughness() const override;
	private:
		Spectrum mSpecularitySpectrum;

		float mIndex;
		float mFresnel;
	};
}