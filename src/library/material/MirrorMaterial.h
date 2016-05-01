#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB MirrorMaterial : public Material
	{
	public:
		MirrorMaterial();

		bool isLight() const;

		void enableCameraVisibility(bool b);
		bool isCameraVisible() const;

		void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer);

	private:
		bool mCameraVisible;
	};
}