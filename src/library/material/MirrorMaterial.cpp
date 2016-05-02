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

	bool MirrorMaterial::shouldIgnore_Simple(const Ray& in, RenderEntity* entity)
	{
		return !mCameraVisible && in.depth() == 0;
	}

	constexpr float NormalOffset = 0.0001f;
	void MirrorMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
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