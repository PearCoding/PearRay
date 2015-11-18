#include "DebugMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/RandomRotationSphere.h"
#include "renderer/Renderer.h"

#include "spectral/RGBConverter.h"

namespace PR
{
	DebugMaterial::DebugMaterial() :
		Material()
	{
	}

	void DebugMaterial::apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer)
	{
		in.setSpectrum(RGBConverter::toSpec(std::fabsf(PM::pm_GetX(point.normal())),
			std::fabsf(PM::pm_GetY(point.normal())),
			std::fabsf(PM::pm_GetZ(point.normal()))));
	}
}