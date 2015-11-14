#include "DebugMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "geometry/RandomRotationSphere.h"
#include "renderer/Renderer.h"

namespace PR
{
	DebugMaterial::DebugMaterial() :
		Material()
	{
	}

	void DebugMaterial::apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer)
	{
		Spectrum spec;
		
		if (PM::pm_GetX(point.normal()) < 0)
		{
			spec.setValue(10, -PM::pm_GetX(point.normal()));
		}
		else
		{
			spec.setValue(15, PM::pm_GetX(point.normal()));
		}

		if (PM::pm_GetY(point.normal()) < 0)
		{
			spec.setValue(20, -PM::pm_GetY(point.normal()));
		}
		else
		{
			spec.setValue(25, PM::pm_GetY(point.normal()));
		}

		if (PM::pm_GetZ(point.normal()) < 0)
		{
			spec.setValue(30, -PM::pm_GetZ(point.normal()));
		}
		else
		{
			spec.setValue(35, PM::pm_GetZ(point.normal()));
		}

		in.setSpectrum(spec);
	}
}