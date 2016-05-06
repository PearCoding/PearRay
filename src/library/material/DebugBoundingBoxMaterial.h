#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB DebugBoundingBoxMaterial : public Material
	{
	public:
		DebugBoundingBoxMaterial();

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li) override;

		float emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;
		float emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;

		float roughness() const override;

		void setColor(const Spectrum& spec);
		Spectrum color() const;

		void setDensity(float f);
		float density() const;
	private:
		Spectrum mColor;
		float mDensity;
	};
}