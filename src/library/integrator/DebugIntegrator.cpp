#include "DebugIntegrator.h"
#include "ray/Ray.h"
#include "shader/SamplePoint.h"
#include "renderer/Renderer.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "spectral/RGBConverter.h"

namespace PR
{
	DebugIntegrator::DebugIntegrator() : OnePassIntegrator()
	{
	}

	void DebugIntegrator::init(Renderer* renderer)
	{
	}

	Spectrum DebugIntegrator::apply(const Ray& in, RenderContext* context)
	{
		Spectrum emission;
		SamplePoint point;
		RenderEntity* entity;
		
		if (context->renderer()->settings().debugMode() == DM_Emission)
			entity = context->shootWithEmission(emission, in, point);
		else
			entity = context->shoot(in, point);

		if (!entity || 
			(context->renderer()->settings().debugMode() != DM_Validity && !point.Material))
			return Spectrum();

		switch (context->renderer()->settings().debugMode())
		{
		default:
		case DM_None:
			return Spectrum();
		case DM_Depth:
		{
			float depth = PM::pm_Magnitude3D(PM::pm_Subtract(in.startPosition(), point.P));
			return RGBConverter::toSpec(depth, depth, depth);
		}
		// NORMAL
		case DM_Normal_Spherical:
		{
			float phi = std::acos(PM::pm_GetZ(point.N));
			float rho = std::atan2(PM::pm_GetY(point.N), PM::pm_GetX(point.N));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Normal_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(point.N)),
				std::abs(PM::pm_GetY(point.N)),
				std::abs(PM::pm_GetZ(point.N)));
		case DM_Normal_Positive:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, PM::pm_GetX(point.N)),
				PM::pm_MaxT(0.0f, PM::pm_GetY(point.N)),
				PM::pm_MaxT(0.0f, PM::pm_GetZ(point.N)));
		case DM_Normal_Negative:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, -PM::pm_GetX(point.N)),
				PM::pm_MaxT(0.0f, -PM::pm_GetY(point.N)),
				PM::pm_MaxT(0.0f, -PM::pm_GetZ(point.N)));
		// TANGENT
		case DM_Tangent_Spherical:
		{
			float phi = std::acos(PM::pm_GetZ(point.Nx));
			float rho = std::atan2(PM::pm_GetY(point.Nx), PM::pm_GetX(point.Nx));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Tangent_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(point.Nx)),
				std::abs(PM::pm_GetY(point.Nx)),
				std::abs(PM::pm_GetZ(point.Nx)));
		case DM_Tangent_Positive:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, PM::pm_GetX(point.Nx)),
				PM::pm_MaxT(0.0f, PM::pm_GetY(point.Nx)),
				PM::pm_MaxT(0.0f, PM::pm_GetZ(point.Nx)));
		case DM_Tangent_Negative:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, -PM::pm_GetX(point.Nx)),
				PM::pm_MaxT(0.0f, -PM::pm_GetY(point.Nx)),
				PM::pm_MaxT(0.0f, -PM::pm_GetZ(point.Nx)));
		// BINORMAL
		case DM_Binormal_Spherical:
		{
			float phi = std::acos(PM::pm_GetZ(point.Ny));
			float rho = std::atan2(PM::pm_GetY(point.Ny), PM::pm_GetX(point.Ny));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Binormal_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(point.Ny)),
				std::abs(PM::pm_GetY(point.Ny)),
				std::abs(PM::pm_GetZ(point.Ny)));
		case DM_Binormal_Positive:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, PM::pm_GetX(point.Ny)),
				PM::pm_MaxT(0.0f, PM::pm_GetY(point.Ny)),
				PM::pm_MaxT(0.0f, PM::pm_GetZ(point.Ny)));
		case DM_Binormal_Negative:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, -PM::pm_GetX(point.N)),
				PM::pm_MaxT(0.0f, -PM::pm_GetY(point.Ny)),
				PM::pm_MaxT(0.0f, -PM::pm_GetZ(point.Ny)));

		case DM_UV:
			return RGBConverter::toSpec(PM::pm_GetX(point.UV), PM::pm_GetY(point.UV), 0);
		case DM_PDF:
		{
			float pdf;
			PM::vec3 rnd = PM::pm_Set(context->random().getFloat(),
				context->random().getFloat(),
				context->random().getFloat());
			PM::vec3 dir = point.Material->sample(point, rnd, pdf);

			return std::isinf(pdf) ? RGBConverter::toSpec(0, 1, 0) : RGBConverter::toSpec(pdf, pdf, pdf);
		}
		case DM_Validity:
		{
			if (!point.Material)
				return RGBConverter::toSpec(1, 0, 0);

			if (PM::pm_MagnitudeSqr3D(point.N) - 1 > PM_EPSILON)
				return RGBConverter::toSpec(1, 1, 0);

			// if (std::abs(PM::pm_GetW(point.N)) > PM_EPSILON)
			// 	return RGBConverter::toSpec(1, 0, 1);

			if (std::abs(PM::pm_GetW(point.P)) - 1 > PM_EPSILON)
				return RGBConverter::toSpec(0, 1, 1);
			
			float pdf;
			PM::vec3 rnd = PM::pm_Set(context->random().getFloat(),
				context->random().getFloat(),
				context->random().getFloat());
			PM::vec3 dir = point.Material->sample(point, rnd, pdf);
			return (std::isinf(pdf) || (pdf > PM_EPSILON && pdf <= 1.0f)) ?
				RGBConverter::toSpec(0, 1, 0) : RGBConverter::toSpec(0, 0, 1);
		}
		case DM_Emission:
			return emission;
		}
	}
}