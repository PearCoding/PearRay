#include "DebugIntegrator.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderThreadContext.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "spectral/RGBConverter.h"

namespace PR
{
	DebugIntegrator::DebugIntegrator() : OnePassIntegrator()
	{
	}

	void DebugIntegrator::init(RenderContext* renderer)
	{
	}

	Spectrum DebugIntegrator::apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc)
	{
		Spectrum emission;
		RenderEntity* entity;

		Ray out = in;
		out.setFlags(in.flags() | RF_Debug);
		
		if (context->renderer()->settings().debugMode() == DM_Emission)
			entity = context->shootWithEmission(emission, out, sc);
		else
			entity = context->shoot(out, sc);

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
			float phi = PM::pm_SafeACos(PM::pm_GetZ(sc.N));
			float rho = std::atan2(PM::pm_GetY(sc.N), PM::pm_GetX(sc.N));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Normal_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(sc.N)),
				std::abs(PM::pm_GetY(sc.N)),
				std::abs(PM::pm_GetZ(sc.N)));
		case DM_Normal_Positive:
			return RGBConverter::toSpec(PM::pm_Max(0.0f, PM::pm_GetX(sc.N)),
				PM::pm_Max(0.0f, PM::pm_GetY(sc.N)),
				PM::pm_Max(0.0f, PM::pm_GetZ(sc.N)));
		case DM_Normal_Negative:
			return RGBConverter::toSpec(PM::pm_Max(0.0f, -PM::pm_GetX(sc.N)),
				PM::pm_Max(0.0f, -PM::pm_GetY(sc.N)),
				PM::pm_Max(0.0f, -PM::pm_GetZ(sc.N)));
		// TANGENT
		case DM_Tangent_Spherical:
		{
			float phi = PM::pm_SafeACos(PM::pm_GetZ(sc.Nx));
			float rho = std::atan2(PM::pm_GetY(sc.Nx), PM::pm_GetX(sc.Nx));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Tangent_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(sc.Nx)),
				std::abs(PM::pm_GetY(sc.Nx)),
				std::abs(PM::pm_GetZ(sc.Nx)));
		case DM_Tangent_Positive:
			return RGBConverter::toSpec(PM::pm_Max(0.0f, PM::pm_GetX(sc.Nx)),
				PM::pm_Max(0.0f, PM::pm_GetY(sc.Nx)),
				PM::pm_Max(0.0f, PM::pm_GetZ(sc.Nx)));
		case DM_Tangent_Negative:
			return RGBConverter::toSpec(PM::pm_Max(0.0f, -PM::pm_GetX(sc.Nx)),
				PM::pm_Max(0.0f, -PM::pm_GetY(sc.Nx)),
				PM::pm_Max(0.0f, -PM::pm_GetZ(sc.Nx)));
		// BINORMAL
		case DM_Binormal_Spherical:
		{
			float phi = PM::pm_SafeACos(PM::pm_GetZ(sc.Ny));
			float rho = std::atan2(PM::pm_GetY(sc.Ny), PM::pm_GetX(sc.Ny));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Binormal_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(sc.Ny)),
				std::abs(PM::pm_GetY(sc.Ny)),
				std::abs(PM::pm_GetZ(sc.Ny)));
		case DM_Binormal_Positive:
			return RGBConverter::toSpec(PM::pm_Max(0.0f, PM::pm_GetX(sc.Ny)),
				PM::pm_Max(0.0f, PM::pm_GetY(sc.Ny)),
				PM::pm_Max(0.0f, PM::pm_GetZ(sc.Ny)));
		case DM_Binormal_Negative:
			return RGBConverter::toSpec(PM::pm_Max(0.0f, -PM::pm_GetX(sc.N)),
				PM::pm_Max(0.0f, -PM::pm_GetY(sc.Ny)),
				PM::pm_Max(0.0f, -PM::pm_GetZ(sc.Ny)));
		//OTHER STUFF
		case DM_UV:
			return RGBConverter::toSpec(PM::pm_GetX(sc.UV), PM::pm_GetY(sc.UV), 0);
		case DM_PDF:
		{
			float full_pdf = 0;
			for(uint32 i = 0; i < 32; ++i)
			{
				float pdf;
				PM::vec3 rnd = PM::pm_Set(context->random().getFloat(),
					context->random().getFloat(),
					context->random().getFloat());
				sc.Material->sample(sc, rnd, pdf);
				full_pdf += pdf;
			}

			full_pdf /= 32;

			return std::isinf(full_pdf) ? RGBConverter::toSpec(0, 1, 0) : RGBConverter::toSpec(full_pdf, full_pdf, full_pdf);
		}
		case DM_Validity:
		{
			/*
			 Color-Code:
			 Red    -> No Material
			 Yellow -> Non unit normal
			 Cyan	-> Position non homogenous
			 Magenta-> Sample Direction non unit
			 Blue   -> PDF out of bounds
			 Green  -> Everything ok
			 Black  -> Background
			*/
			
			if (!sc.Material)
				return RGBConverter::toSpec(1, 0, 0);

			if (PM::pm_MagnitudeSqr3D(sc.N) - 1 > PM_EPSILON)
				return RGBConverter::toSpec(1, 1, 0);

			if (std::abs(PM::pm_GetW(sc.P)) - 1 > PM_EPSILON)
				return RGBConverter::toSpec(0, 1, 1);
			
			float pdf;
			PM::vec3 rnd = PM::pm_Set(context->random().getFloat(),
				context->random().getFloat(),
				context->random().getFloat());
			PM::vec3 dir = sc.Material->sample(sc, rnd, pdf);
			
			if (PM::pm_MagnitudeSqr3D(dir) - 1 > PM_EPSILON)
				return RGBConverter::toSpec(1, 0, 1);

			return (std::isinf(pdf) || (pdf > PM_EPSILON && pdf <= 1.0f)) ?
				RGBConverter::toSpec(0, 1, 0) : RGBConverter::toSpec(0, 0, 1);
		}
		case DM_Emission:
			return emission;
		}
	}
}