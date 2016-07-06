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

	Spectrum DebugIntegrator::apply(const Ray& in, RenderContext* context)
	{
		Spectrum applied;
		FacePoint point;
		RenderEntity* entity;
		
		if (context->renderer()->settings().debugMode() == DM_Applied)
			entity = context->shootWithApply(applied, in, point);
		else
			entity = context->shoot(in, point);

		if (!entity || 
			(context->renderer()->settings().debugMode() != DM_Validity && !point.material()))
			return Spectrum();

		switch (context->renderer()->settings().debugMode())
		{
		default:
		case DM_None:
			return Spectrum();
		case DM_Depth:
		{
			float depth = PM::pm_Magnitude3D(PM::pm_Subtract(in.startPosition(), point.vertex()));
			return RGBConverter::toSpec(depth, depth, depth);
		}
		// NORMAL
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
		// TANGENT
		case DM_Tangent_Spherical:
		{
			float phi = std::acos(PM::pm_GetZ(point.tangent()));
			float rho = std::atan2(PM::pm_GetY(point.tangent()), PM::pm_GetX(point.tangent()));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Tangent_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(point.tangent())),
				std::abs(PM::pm_GetY(point.tangent())),
				std::abs(PM::pm_GetZ(point.tangent())));
		case DM_Tangent_Positive:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, PM::pm_GetX(point.tangent())),
				PM::pm_MaxT(0.0f, PM::pm_GetY(point.tangent())),
				PM::pm_MaxT(0.0f, PM::pm_GetZ(point.tangent())));
		case DM_Tangent_Negative:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, -PM::pm_GetX(point.tangent())),
				PM::pm_MaxT(0.0f, -PM::pm_GetY(point.tangent())),
				PM::pm_MaxT(0.0f, -PM::pm_GetZ(point.tangent())));
		// BINORMAL
		case DM_Binormal_Spherical:
		{
			float phi = std::acos(PM::pm_GetZ(point.binormal()));
			float rho = std::atan2(PM::pm_GetY(point.binormal()), PM::pm_GetX(point.binormal()));

			return RGBConverter::toSpec(phi * PM_INV_PI_F, rho * PM_INV_PI_F * 0.5f, 0);
		}
		case DM_Binormal_Both:
			return RGBConverter::toSpec(std::abs(PM::pm_GetX(point.binormal())),
				std::abs(PM::pm_GetY(point.binormal())),
				std::abs(PM::pm_GetZ(point.binormal())));
		case DM_Binormal_Positive:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, PM::pm_GetX(point.binormal())),
				PM::pm_MaxT(0.0f, PM::pm_GetY(point.binormal())),
				PM::pm_MaxT(0.0f, PM::pm_GetZ(point.binormal())));
		case DM_Binormal_Negative:
			return RGBConverter::toSpec(PM::pm_MaxT(0.0f, -PM::pm_GetX(point.binormal())),
				PM::pm_MaxT(0.0f, -PM::pm_GetY(point.binormal())),
				PM::pm_MaxT(0.0f, -PM::pm_GetZ(point.binormal())));

		case DM_UV:
			return RGBConverter::toSpec(PM::pm_GetX(point.uv()), PM::pm_GetY(point.uv()), 0);
		case DM_PDF:
		{
			float pdf;
			PM::vec3 rnd = PM::pm_Set(context->random().getFloat(),
				context->random().getFloat(),
				context->random().getFloat());
			PM::vec3 dir = point.material()->sample(point, rnd, in.direction(), pdf);

			return std::isinf(pdf) ? RGBConverter::toSpec(0, 1, 0) : RGBConverter::toSpec(pdf, pdf, pdf);
		}
		case DM_Validity:
		{
			if (!point.material())
				return RGBConverter::toSpec(1, 0, 0);

			float pdf;
			PM::vec3 rnd = PM::pm_Set(context->random().getFloat(),
				context->random().getFloat(),
				context->random().getFloat());
			PM::vec3 dir = point.material()->sample(point, rnd, in.direction(), pdf);
			return (std::isinf(pdf) || (pdf > PM_EPSILON && pdf <= 1.0f)) ?
				RGBConverter::toSpec(0, 1, 0) : RGBConverter::toSpec(0, 0, 1);
		}
		case DM_Applied:
			return applied;
		}
	}
}