#include "DebugIntegrator.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "shader/ShaderClosure.h"
#include "spectral/RGBConverter.h"

namespace PR {
DebugIntegrator::DebugIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
{
}

void DebugIntegrator::init()
{
}

Spectrum DebugIntegrator::apply(const Ray& in, RenderTile* tile, uint32 pass, ShaderClosure& sc)
{
	Spectrum emission;
	RenderEntity* entity;

	Ray out = in;
	out.setFlags(in.flags() | RF_Debug);

	if (renderer()->settings().debugMode() == DM_Emission)
		entity = renderer()->shootWithEmission(emission, out, sc, tile);
	else
		entity = renderer()->shoot(out, sc, tile);

	if (!entity || (renderer()->settings().debugMode() != DM_Validity && !sc.Material))
		return Spectrum();

	switch (renderer()->settings().debugMode()) {
	default:
	case DM_None:
		return Spectrum();
	case DM_Depth: {
		float depth = (in.origin() - sc.P).norm();
		return RGBConverter::toSpec(depth, depth, depth);
	}
	// NORMAL
	case DM_Normal_Spherical: {
		float phi = std::acos(sc.N(2));
		float rho = std::atan2(sc.N(1), sc.N(0));

		return RGBConverter::toSpec(phi * PR_1_PI, rho * PR_1_PI * 0.5f, 0);
	}
	case DM_Normal_Both:
		return RGBConverter::toSpec(std::abs(sc.N(0)),
									std::abs(sc.N(1)),
									std::abs(sc.N(2)));
	case DM_Normal_Positive:
		return RGBConverter::toSpec(std::max(0.0f, sc.N(0)),
									std::max(0.0f, sc.N(1)),
									std::max(0.0f, sc.N(2)));
	case DM_Normal_Negative:
		return RGBConverter::toSpec(std::max(0.0f, -sc.N(0)),
									std::max(0.0f, -sc.N(1)),
									std::max(0.0f, -sc.N(2)));
	// TANGENT
	case DM_Tangent_Spherical: {
		float phi = std::acos(sc.Nx(2));
		float rho = std::atan2(sc.Nx(1), sc.Nx(0));

		return RGBConverter::toSpec(phi * PR_1_PI, rho * PR_1_PI * 0.5f, 0);
	}
	case DM_Tangent_Both:
		return RGBConverter::toSpec(std::abs(sc.Nx(0)),
									std::abs(sc.Nx(1)),
									std::abs(sc.Nx(2)));
	case DM_Tangent_Positive:
		return RGBConverter::toSpec(std::max(0.0f, sc.Nx(0)),
									std::max(0.0f, sc.Nx(1)),
									std::max(0.0f, sc.Nx(2)));
	case DM_Tangent_Negative:
		return RGBConverter::toSpec(std::max(0.0f, -sc.Nx(0)),
									std::max(0.0f, -sc.Nx(1)),
									std::max(0.0f, -sc.Nx(2)));
	// BINORMAL
	case DM_Binormal_Spherical: {
		float phi = std::acos(sc.Ny(2));
		float rho = std::atan2(sc.Ny(1), sc.Ny(0));

		return RGBConverter::toSpec(phi * PR_1_PI, rho * PR_1_PI * 0.5f, 0);
	}
	case DM_Binormal_Both:
		return RGBConverter::toSpec(std::abs(sc.Ny(0)),
									std::abs(sc.Ny(1)),
									std::abs(sc.Ny(2)));
	case DM_Binormal_Positive:
		return RGBConverter::toSpec(std::max(0.0f, sc.Ny(0)),
									std::max(0.0f, sc.Ny(1)),
									std::max(0.0f, sc.Ny(2)));
	case DM_Binormal_Negative:
		return RGBConverter::toSpec(std::max(0.0f, -sc.N(0)),
									std::max(0.0f, -sc.Ny(1)),
									std::max(0.0f, -sc.Ny(2)));
	//OTHER STUFF
	case DM_UVW:
		return RGBConverter::toSpec(sc.UVW(0), sc.UVW(1), sc.UVW(2));
	case DM_PDF: {
		float full_pdf = 0;
		for (uint32 i = 0; i < 32; ++i) {
			Eigen::Vector3f rnd = tile->random().get3D();
			MaterialSample ms = sc.Material->sample(sc, rnd);
			full_pdf += ms.PDF;
		}

		full_pdf /= 32;

		return std::isinf(full_pdf) ? RGBConverter::toSpec(0, 1, 0) : RGBConverter::toSpec(full_pdf, full_pdf, full_pdf);
	}
	case DM_Validity: {
		/*
			 Color-Code:
			 Red    -> No Material
			 Yellow -> Non unit normal
			 Magenta-> Sample Direction non unit
			 Blue   -> PDF out of bounds
			 Green  -> Everything ok
			 Black  -> Background
			*/

		if (!sc.Material)
			return RGBConverter::toSpec(1, 0, 0);

		if (sc.N.squaredNorm() - 1 > PR_EPSILON)
			return RGBConverter::toSpec(1, 1, 0);

		Eigen::Vector3f rnd = tile->random().get3D();
		MaterialSample ms = sc.Material->sample(sc, rnd);

		if (ms.L.squaredNorm() - 1 > PR_EPSILON)
			return RGBConverter::toSpec(1, 0, 1);

		return (std::isinf(ms.PDF) || (ms.PDF > PR_EPSILON && ms.PDF <= 1.0f)) ? RGBConverter::toSpec(0, 1, 0) : RGBConverter::toSpec(0, 0, 1);
	}
	case DM_Flag_Inside:
		if (!sc.Flags & SCF_Inside)
			return RGBConverter::toSpec(0, 1, 0);
		else
			return RGBConverter::toSpec(1, 0, 0);
	case DM_Emission:
		return emission;
	case DM_Container_ID:
		return RGBConverter::toSpec(entity->containerID(), entity->containerID(), entity->containerID());
	}
}
}
