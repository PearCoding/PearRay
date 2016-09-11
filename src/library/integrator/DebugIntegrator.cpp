#include "DebugIntegrator.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
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

	Spectrum DebugIntegrator::apply(const Ray& in, RenderContext* context, uint32 pass)
	{
		Spectrum emission;
		ShaderClosure sc;
		RenderEntity* entity;
		
		if (context->renderer()->settings().debugMode() == DM_Emission)
			entity = context->shootWithEmission(emission, in, sc);
		else
			entity = context->shoot(in, sc);

		if (!entity || 
			(context->renderer()->settings().debugMode() != DM_Validity && !sc.Material))
			return Spectrum();

		switch (context->renderer()->settings().debugMode())
		{
		default:
		case DM_None:
			return Spectrum();
		case DM_Depth:
		{
			float depth = PM::pm_Magnitude3D(PM::pm_Subtract(in.startPosition(), sc.P));
			return RGBConverter::toSpec(depth, depth, depth);
		}
		// NORMAL
		case DM_Normal_Spherical:
		{
			float phi = std::acos(PM::pm_GetZ(sc.N));
			float rho = std::atan2(PM::pm_GetY(sc.N), PM::pm_GetX(sc.N));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Normal_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(sc.N)),
				std::abs(PM::pm_GetY(sc.N)),
				std::abs(PM::pm_GetZ(sc.N)));
		case DM_Normal_Positive:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, PM::pm_GetX(sc.N)),
				PM::pm_MaxT(0.0f, PM::pm_GetY(sc.N)),
				PM::pm_MaxT(0.0f, PM::pm_GetZ(sc.N)));
		case DM_Normal_Negative:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, -PM::pm_GetX(sc.N)),
				PM::pm_MaxT(0.0f, -PM::pm_GetY(sc.N)),
				PM::pm_MaxT(0.0f, -PM::pm_GetZ(sc.N)));
		// TANGENT
		case DM_Tangent_Spherical:
		{
			float phi = std::acos(PM::pm_GetZ(sc.Nx));
			float rho = std::atan2(PM::pm_GetY(sc.Nx), PM::pm_GetX(sc.Nx));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Tangent_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(sc.Nx)),
				std::abs(PM::pm_GetY(sc.Nx)),
				std::abs(PM::pm_GetZ(sc.Nx)));
		case DM_Tangent_Positive:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, PM::pm_GetX(sc.Nx)),
				PM::pm_MaxT(0.0f, PM::pm_GetY(sc.Nx)),
				PM::pm_MaxT(0.0f, PM::pm_GetZ(sc.Nx)));
		case DM_Tangent_Negative:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, -PM::pm_GetX(sc.Nx)),
				PM::pm_MaxT(0.0f, -PM::pm_GetY(sc.Nx)),
				PM::pm_MaxT(0.0f, -PM::pm_GetZ(sc.Nx)));
		// BINORMAL
		case DM_Binormal_Spherical:
		{
			float phi = std::acos(PM::pm_GetZ(sc.Ny));
			float rho = std::atan2(PM::pm_GetY(sc.Ny), PM::pm_GetX(sc.Ny));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Binormal_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(sc.Ny)),
				std::abs(PM::pm_GetY(sc.Ny)),
				std::abs(PM::pm_GetZ(sc.Ny)));
		case DM_Binormal_Positive:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, PM::pm_GetX(sc.Ny)),
				PM::pm_MaxT(0.0f, PM::pm_GetY(sc.Ny)),
				PM::pm_MaxT(0.0f, PM::pm_GetZ(sc.Ny)));
		case DM_Binormal_Negative:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, -PM::pm_GetX(sc.N)),
				PM::pm_MaxT(0.0f, -PM::pm_GetY(sc.Ny)),
				PM::pm_MaxT(0.0f, -PM::pm_GetZ(sc.Ny)));

		case DM_UV:
			return RGBConverter::toSpec(PM::pm_GetX(sc.UV), PM::pm_GetY(sc.UV), 0);
		case DM_PDF:
		{
			float pdf;
			PM::vec3 rnd = PM::pm_Set(context->random().getFloat(),
				context->random().getFloat(),
				context->random().getFloat());
			sc.Material->sample(sc, rnd, pdf);

			return std::isinf(pdf) ? RGBConverter::toSpec(0, 1, 0) : RGBConverter::toSpec(pdf, pdf, pdf);
		}
		case DM_Validity:
		{
			if (!sc.Material)
				return RGBConverter::toSpec(1, 0, 0);

			if (PM::pm_MagnitudeSqr3D(sc.N) - 1 > PM_EPSILON)
				return RGBConverter::toSpec(1, 1, 0);

			// if (std::abs(PM::pm_GetW(sc.N)) > PM_EPSILON)
			// 	return RGBConverter::toSpec(1, 0, 1);

			if (std::abs(PM::pm_GetW(sc.P)) - 1 > PM_EPSILON)
				return RGBConverter::toSpec(0, 1, 1);
			
			float pdf;
			PM::vec3 rnd = PM::pm_Set(context->random().getFloat(),
				context->random().getFloat(),
				context->random().getFloat());
			sc.Material->sample(sc, rnd, pdf);
			return (std::isinf(pdf) || (pdf > PM_EPSILON && pdf <= 1.0f)) ?
				RGBConverter::toSpec(0, 1, 0) : RGBConverter::toSpec(0, 0, 1);
		}
		case DM_Emission:
			return emission;
		}
	}
}