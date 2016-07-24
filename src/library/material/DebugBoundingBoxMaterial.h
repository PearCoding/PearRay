#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB DebugBoundingBoxMaterial : public Material
	{
	public:
		DebugBoundingBoxMaterial();

		Spectrum apply(const SamplePoint& point, const PM::vec3& L) override;
		float pdf(const SamplePoint& point, const PM::vec3& L) override;
		PM::vec3 sample(const SamplePoint& point, const PM::vec3& rnd, float& pdf) override;

		void setColor(const Spectrum& spec);
		Spectrum color() const;

		void setDensity(float f);
		float density() const;
	private:
		Spectrum mColor;
		float mDensity;
	};
}