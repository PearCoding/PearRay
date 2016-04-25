#include "UVDebugMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/RandomRotationSphere.h"
#include "renderer/Renderer.h"

#include "spectral/RGBConverter.h"

namespace PR
{
	UVDebugMaterial::UVDebugMaterial() :
		Material()
	{
	}

	void UVDebugMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
		in.setSpectrum(RGBConverter::toSpec(std::fabsf(PM::pm_GetX(point.uv())),
			std::fabsf(PM::pm_GetY(point.uv())),
			0));
	}

	bool UVDebugMaterial::isLight() const
	{
		return false;
	}
}