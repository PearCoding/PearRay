#include "Renderer.h"
#include "scene/Camera.h"
#include "scene/Scene.h"
#include "ray/Ray.h"

namespace PR
{
	Renderer::Renderer(uint32 w, uint32 h) :
		mWidth(w), mHeight(h)
	{
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::setWidth(uint32 w)
	{
		mWidth = w;
	}

	uint32 Renderer::width() const
	{
		return mWidth;
	}

	void Renderer::setHeight(uint32 h)
	{
		mHeight = h;
	}

	uint32 Renderer::height() const
	{
		return mHeight;
	}

	RenderResult Renderer::render(Camera* cam, Scene* scene)
	{
		PR_ASSERT(cam);
		PR_ASSERT(scene);

		mRayCount = 0;

		RenderResult result(mWidth, mHeight);

		for (uint32 y = 0; y < mHeight; ++y)
		{
			for (uint32 x = 0; x < mWidth; ++x)
			{
				float sx = cam->width() * x / (float) mWidth - cam->width() / 2.0f;
				float sy = cam->height() * y / (float) mHeight - cam->height() / 2.0f;

				Ray ray(PM::pm_Multiply(cam->matrix(), PM::pm_Set(sx, sy, 0)),
					PM::pm_Multiply(PM::pm_Rotation(cam->rotation()), PM::pm_Set(0, 0, 1)));

				PM::vec3 collisionPoint;
				Entity* entity = scene->checkCollision(ray, collisionPoint);

				if (!entity)
				{
					result.setPoint(x, y, 0);
				}
				else
				{
					result.setPoint(x, y, 1);
				}

				mRayCount++;
			}
		}
		return result;
	}

	size_t Renderer::rayCount() const
	{
		return mRayCount;
	}
}