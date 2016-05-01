#include "MirrorMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "sampler/Stratified2DSampler.h"
#include "sampler/Stratified3DSampler.h"
#include "sampler/Projection.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

#include "BRDF.h"

namespace PR
{
	MirrorMaterial::MirrorMaterial() :
		Material(), 
		mCameraVisible(true)
	{
	}

	bool MirrorMaterial::isLight() const
	{
		return false;
	}

	void MirrorMaterial::enableCameraVisibility(bool b)
	{
		mCameraVisible = b;
	}

	bool MirrorMaterial::isCameraVisible() const
	{
		return mCameraVisible;
	}

	constexpr float NormalOffset = 0.0001f;
	void MirrorMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
		const uint32 maxDepth = in.maxDepth() == 0 ?
			renderer->maxRayDepth() : PM::pm_MinT<uint32>(renderer->maxRayDepth() + 1, in.maxDepth());
		if (in.depth() < maxDepth && (mCameraVisible || in.depth() > 0))
		{
			PM::vec3 reflect = PM::pm_Subtract(in.direction(),
				PM::pm_Scale(point.normal(), 2 * PM::pm_Dot3D(in.direction(), point.normal())));
			Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(reflect, NormalOffset)), reflect, in.depth() + 1);

			FacePoint collisionPoint;
			if (renderer->shoot(ray, collisionPoint))
			{
				in.setSpectrum(ray.spectrum());
			}
		}
	}
}