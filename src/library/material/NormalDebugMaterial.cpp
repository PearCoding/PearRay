#include "NormalDebugMaterial.h"
#include "geometry/FacePoint.h"

#include "spectral/RGBConverter.h"

namespace PR
{
	NormalDebugMaterial::NormalDebugMaterial() :
		Material()
	{
	}

	void NormalDebugMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li,
		Spectrum& diff, Spectrum& spec)
	{
		diff = RGBConverter::toSpec(std::abs(PM::pm_GetX(point.normal())),
			std::abs(PM::pm_GetY(point.normal())),
			std::abs(PM::pm_GetZ(point.normal())));
	}

	float NormalDebugMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		return 0;
	}

	float NormalDebugMaterial::emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		return 0;
	}

	float NormalDebugMaterial::roughness() const
	{
		return 1;
	}
}