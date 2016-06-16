#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB DebugBoundingBoxMaterial : public Material
	{
	public:
		DebugBoundingBoxMaterial();

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		float pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		PM::vec3 sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf) override;

		void setColor(const Spectrum& spec);
		Spectrum color() const;

		void setDensity(float f);
		float density() const;
	private:
		Spectrum mColor;
		float mDensity;
	};
}