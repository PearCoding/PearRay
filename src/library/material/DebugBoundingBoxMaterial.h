#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB DebugBoundingBoxMaterial : public Material
	{
	public:
		DebugBoundingBoxMaterial();

		void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer);
		bool isLight() const;

		void setColor(const Spectrum& spec);
		Spectrum color() const;

		void setDensity(float f);
		float density() const;
	private:
		Spectrum mColor;
		float mDensity;
	};
}