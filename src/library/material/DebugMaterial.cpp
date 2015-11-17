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
		//in.setSpectrum(RGBConverter::Red);
		//in.setSpectrum(RGBConverter::toSpec(0.5f, 0.5f, 0.5f));

		Spectrum spec;
		spec.setValueAtWavelength(430, 1);
		in.setSpectrum(spec);

		//if (PM::pm_GetX(point.normal()) < 0)
		//{
		//	in.setSpectrum(converter.toSpec(1/*-PM::pm_GetX(point.normal())*/, 0, 0));
		//}
		//else
		//{
		//	in.setSpectrum(converter.toSpec(0, 1/*PM::pm_GetX(point.normal())*/, 0));
		//}

		//if (PM::pm_GetY(point.normal()) < 0)
		//{
		//	//spec.setValue(20, -PM::pm_GetY(point.normal()));
		//}
		//else
		//{
		//	//spec.setValue(25, PM::pm_GetY(point.normal()));
		//}

		//if (PM::pm_GetZ(point.normal()) < 0)
		//{
		//	//spec.setValue(30, -PM::pm_GetZ(point.normal()));
		//}
		//else
		//{
		//	//spec.setValue(35, PM::pm_GetZ(point.normal()));
		//}
	}
}