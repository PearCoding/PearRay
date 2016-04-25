#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB UVDebugMaterial : public Material
	{
	public:
		UVDebugMaterial();

		void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer);
		bool isLight() const;
	};
}