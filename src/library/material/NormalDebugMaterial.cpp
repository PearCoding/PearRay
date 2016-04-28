#include "NormalDebugMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/RandomRotationSphere.h"
#include "renderer/Renderer.h"

#include "spectral/RGBConverter.h"

namespace PR
{
	NormalDebugMaterial::NormalDebugMaterial() :
		Material()
	{
	}

	void NormalDebugMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
		in.setSpectrum(RGBConverter::toSpec(std::abs(PM::pm_GetX(point.normal())),
			std::abs(PM::pm_GetY(point.normal())),
			std::abs(PM::pm_GetZ(point.normal()))));
	}

	bool NormalDebugMaterial::isLight() const
	{
		return false;
	}
}