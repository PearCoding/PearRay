#include "DirectLightDebugMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/RandomRotationSphere.h"
#include "renderer/Renderer.h"

#include "entity/RenderEntity.h"

#include "spectral/RGBConverter.h"

namespace PR
{
	DirectLightDebugMaterial::DirectLightDebugMaterial() :
		Material()
	{
	}

	void DirectLightDebugMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
		bool found = false;

		FacePoint collisionPoint;
		for (RenderEntity* light : renderer->lights())
		{
			uint32 max = renderer->maxDirectRayCount();
			max = light->maxLightSamples() != 0 ? PM::pm_MinT(max, light->maxLightSamples()) : max;

			for (uint32 i = 0; i < max; ++i)
			{
				FacePoint p = light->getRandomFacePoint(renderer->random());
				PM::vec3 dir = PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex()));

				Ray ray(point.vertex(), dir, in.depth() + 1);// Bounce only once!
				ray.setFlags(0);
				ray.setMaxDepth(in.depth() + 1);

				RenderEntity* collidedEntity = renderer->shoot(ray, collisionPoint, entity);

				if (collidedEntity == light)// Full light!!
				{
					float dot2 = PM::pm_Dot3D(dir, point.normal());

					if (dot2 > 0)
					{
						found = true;
						break;
					}
				}
			}

			if (found)
				break;
		}

		if (!found)
		{
			in.setSpectrum(RGBConverter::toSpec(0,0,0));
		}
		else
		{
			in.setSpectrum(RGBConverter::toSpec(1,1,1));
		}
	}

	bool DirectLightDebugMaterial::isLight() const
	{
		return false;
	}
}