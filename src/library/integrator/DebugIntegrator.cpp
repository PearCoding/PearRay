#include "DebugIntegrator.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "spectral/RGBConverter.h"

namespace PR
{
	DebugIntegrator::DebugIntegrator() : Integrator()
	{
	}

	void DebugIntegrator::init(Renderer* renderer)
	{
	}

	Spectrum DebugIntegrator::apply(Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context)
	{
		switch (context->renderer()->settings().debugMode())
		{
		default:
		case DM_None:
			return Spectrum();
		case DM_Normal_Spherical:
		{
			float phi = std::acos(PM::pm_GetZ(point.normal()));
			float rho = std::atan2(PM::pm_GetY(point.normal()), PM::pm_GetX(point.normal()));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Normal_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(point.normal())),
				std::abs(PM::pm_GetY(point.normal())),
				std::abs(PM::pm_GetZ(point.normal())));
		case DM_Normal_Positive:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, PM::pm_GetX(point.normal())),
				PM::pm_MaxT(0.0f, PM::pm_GetY(point.normal())),
				PM::pm_MaxT(0.0f, PM::pm_GetZ(point.normal())));
		case DM_Normal_Negative:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, -PM::pm_GetX(point.normal())),
				PM::pm_MaxT(0.0f, -PM::pm_GetY(point.normal())),
				PM::pm_MaxT(0.0f, -PM::pm_GetZ(point.normal())));
		case DM_UV:
			return RGBConverter::toSpec(PM::pm_GetX(point.uv()), PM::pm_GetY(point.uv()), 0);
		case DM_Roughness:
		{
			float roughness = entity->material()->roughness(point);
			return RGBConverter::toSpec(roughness, roughness, roughness);
		}
		case DM_Reflectivity:
		{
			PM::vec3 tmp;
			float weight = entity->material()->emitReflectionVector(point, in.direction(), tmp);
			return RGBConverter::toSpec(weight, weight, weight);
		}
		case DM_Transmission:
		{
			PM::vec3 tmp;
			float weight = entity->material()->emitTransmissionVector(point, in.direction(), tmp);
			return RGBConverter::toSpec(weight, weight, weight);
		}
		}
	}
}