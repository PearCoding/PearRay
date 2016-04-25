#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB NormalDebugMaterial : public Material
	{
	public:
		NormalDebugMaterial();

		void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer);
		bool isLight() const;
	};
}