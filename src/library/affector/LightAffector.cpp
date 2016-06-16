#include "LightAffector.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"
#include "sampler/StratifiedSampler.h"
#include "material/Material.h"

namespace PR
{
	LightAffector::LightAffector() : Affector()
	{
	}

	void LightAffector::init(Renderer* renderer)
	{
	}

	Spectrum LightAffector::apply(const Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context)
	{
		if (point.material()->isLight())
			return point.material()->applyEmission(point, in.direction());
		else
			return Spectrum();
	}
}