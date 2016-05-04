#include "UVDebugMaterial.h"
#include "geometry/FacePoint.h"

#include "spectral/RGBConverter.h"

namespace PR
{
	UVDebugMaterial::UVDebugMaterial() :
		Material()
	{
	}

	void UVDebugMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li,
		Spectrum& diff, Spectrum& spec)
	{
		diff = RGBConverter::toSpec(std::abs(PM::pm_GetX(point.uv())),
			std::abs(PM::pm_GetY(point.uv())),
			0);
	}

	float UVDebugMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		return 0;
	}

	float UVDebugMaterial::emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		return 0;
	}

	float UVDebugMaterial::roughness() const
	{
		return 1;
	}
}