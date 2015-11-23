#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB DebugMaterial : public Material
	{
	public:
		DebugMaterial();

		void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer);
		bool isLight() const;
	};
}