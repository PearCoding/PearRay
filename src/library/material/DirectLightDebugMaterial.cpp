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
		if (entity->isLight())
		{
			in.setSpectrum(RGBConverter::toSpec(1, 1, 0));
			return;
		}

		FacePoint collisionPoint;
		int lightSamples = 0;
		float intensity = 0;
		for (RenderEntity* light : renderer->lights())
		{
			uint32 max = renderer->maxDirectRayCount();
			max = light->maxLightSamples() != 0 ? PM::pm_MinT(max, light->maxLightSamples()) : max;

			for (uint32 i = 0; i < max; ++i)
			{
				FacePoint p = light->getRandomFacePoint(renderer->random());
				PM::vec3 dir = PM::pm_SetW(PM::pm_Normalize3D(PM::pm_Subtract(p.vertex(), point.vertex())), 0);

				float dot = PM::pm_Dot3D(dir, point.normal());
				if (std::abs(dot) > std::numeric_limits<float>::epsilon())
				{
					Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(dir, 0.00001f)), dir, in.depth() + 1);// Bounce only once!
					//ray.setFlags(0);
					ray.setMaxDepth(in.depth() + 1);

					RenderEntity* collidedEntity = renderer->shoot(ray, collisionPoint);

					if (collidedEntity == light)// Full light!!
					{
						intensity += std::abs(dot);
						lightSamples++;
						break;
					}
				}
			}
		}

		if (lightSamples == 0)
		{
			in.setSpectrum(RGBConverter::toSpec(0,0,0));
		}
		else
		{
			intensity /= lightSamples;
			in.setSpectrum(RGBConverter::toSpec(intensity, intensity, intensity));
		}
	}

	bool DirectLightDebugMaterial::isLight() const
	{
		return false;
	}
}