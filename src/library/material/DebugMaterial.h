#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB DebugMaterial : public Material
	{
	public:
		DebugMaterial();

		void apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer);
	};
}