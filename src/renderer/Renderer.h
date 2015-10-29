#pragma once

#include "RenderResult.h"

namespace PR
{
	class Camera;
	class Scene;
	class Renderer
	{
	public:
		Renderer(uint32 width, uint32 height);
		virtual ~Renderer();

		void setWidth(uint32 w);
		uint32 width() const;

		void setHeight(uint32 h);
		uint32 height() const;

		RenderResult render(Camera* cam, Scene* scene);

		size_t rayCount() const;
	private:
		uint32 mWidth;
		uint32 mHeight;

		size_t mRayCount;
	};
}