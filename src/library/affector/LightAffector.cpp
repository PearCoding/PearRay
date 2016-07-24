#include "LightAffector.h"
#include "ray/Ray.h"
#include "shader/SamplePoint.h"
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

	Spectrum LightAffector::apply(const Ray& in, RenderEntity* entity, const SamplePoint& point, RenderContext* context)
	{
		if (point.Material->isLight())
			return point.Material->applyEmission(point);
		else
			return Spectrum();
	}
}