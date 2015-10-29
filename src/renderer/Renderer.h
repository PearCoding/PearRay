#pragma once

#include "RenderResult.h"

namespace PR
{
	class Camera;
	class Scene;
	class Renderer
	{
	public:
		Renderer();
		virtual ~Renderer();

		RenderResult render(Camera* cam, Scene* scene);

		size_t rayCount() const;
	private:
		size_t mRayCount;
	};
}