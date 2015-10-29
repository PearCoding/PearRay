#include "Renderer.h"
#include "scene/Camera.h"
#include "scene/Scene.h"

namespace PR
{
	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}

	RenderResult Renderer::render(Camera* cam, Scene* scene)
	{
		PR_ASSERT(cam);
		PR_ASSERT(scene);

		RenderResult result(cam);

		for (uint32 y = 0; y < cam->height(); ++y)
		{
			for (uint32 x = 0; x < cam->width(); ++x)
			{

			}
		}
		return result;
	}

	size_t Renderer::rayCount() const
	{

	}
}