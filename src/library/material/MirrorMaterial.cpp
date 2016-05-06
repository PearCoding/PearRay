#include "MirrorMaterial.h"
#include "geometry/FacePoint.h"

#include "BRDF.h"

namespace PR
{
	MirrorMaterial::MirrorMaterial() :
		Material()
	{
	}

	Spectrum MirrorMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li)
	{
		// TODO
		return Li;
	}

	float MirrorMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		dir = BRDF::reflect(point.normal(), V);
		return 1;
	}

	float MirrorMaterial::emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		return 0;
	}

	float MirrorMaterial::roughness() const
	{
		return 0;
	}
}