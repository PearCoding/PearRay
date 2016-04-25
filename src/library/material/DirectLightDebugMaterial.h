#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB DirectLightDebugMaterial : public Material
	{
	public:
		DirectLightDebugMaterial();

		void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer);
		bool isLight() const;
	};
}