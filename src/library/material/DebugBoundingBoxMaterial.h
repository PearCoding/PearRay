#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB DebugBoundingBoxMaterial : public Material
	{
	public:
		DebugBoundingBoxMaterial();

		Spectrum apply(const ShaderClosure& point, const PM::vec3& L) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;

		void setColor(const Spectrum& spec);
		Spectrum color() const;

		void setDensity(float f);
		float density() const;
	private:
		Spectrum mColor;
		float mDensity;
	};
}