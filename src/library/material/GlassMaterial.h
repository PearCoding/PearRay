#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB GlassMaterial : public Material
	{
	public:
		GlassMaterial();

		Texture2D* specularity() const;
		void setSpecularity(Texture2D* spec);

		float index() const;
		void setIndex(float f);

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li) override;

		float emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;
		float emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;

		float roughness(const FacePoint& point) const override;
	private:
		Texture2D* mSpecularity;

		float mIndex;
		float mFresnel;
	};
}