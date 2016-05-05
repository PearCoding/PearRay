#include "LightIntegrator.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "sampler/Projection.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"
#include "sampler/StratifiedSampler.h"
#include "material/Material.h"

namespace PR
{
	LightIntegrator::LightIntegrator() : Integrator()
	{
	}

	void LightIntegrator::init(Renderer* renderer)
	{
	}

	Spectrum LightIntegrator::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
		if (entity->material()->isLight())
			return entity->material()->applyEmission(point, in.direction());
		else
			return Spectrum();
	}
}