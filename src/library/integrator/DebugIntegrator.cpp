#include "DebugIntegrator.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "ray/Ray.h"

#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderTile.h"

#include "shader/ShaderClosure.h"
#include "spectral/RGBConverter.h"

namespace PR {
DebugIntegrator::DebugIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
	, mDebugMode(renderer->settings().debugMode())
{
}

void DebugIntegrator::init()
{
	OnePassIntegrator::init();
}

void DebugIntegrator::onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session)
{
	RenderEntity* entity;

	Ray out = in;
	out.setFlags(in.flags() | RF_Debug);

	spec.clear();

	if (mDebugMode == DM_Emission)
		entity = renderer()->shootWithEmission(spec, out, sc, session);
	else
		entity = renderer()->shoot(out, sc, session);

	if (!entity || (mDebugMode != DM_Validity && !sc.Material))
		return;

	switch (mDebugMode) {
	default:
	case DM_None:
		break;
	case DM_Depth: {
		float depth = (in.origin() - sc.P).norm();
		RGBConverter::toSpec(spec, depth, depth, depth);
	} break;
	// NORMAL
	case DM_Normal_Spherical: {
		float phi = std::acos(sc.N(2));
		float rho = std::atan2(sc.N(1), sc.N(0));

		RGBConverter::toSpec(spec, phi * PR_1_PI, rho * PR_1_PI * 0.5f, 0);
	} break;
	case DM_Normal_Both:
		RGBConverter::toSpec(spec, std::abs(sc.N(0)),
							 std::abs(sc.N(1)),
							 std::abs(sc.N(2)));
		break;
	case DM_Normal_Positive:
		RGBConverter::toSpec(spec, std::max(0.0f, sc.N(0)),
							 std::max(0.0f, sc.N(1)),
							 std::max(0.0f, sc.N(2)));
		break;
	case DM_Normal_Negative:
		RGBConverter::toSpec(spec, std::max(0.0f, -sc.N(0)),
							 std::max(0.0f, -sc.N(1)),
							 std::max(0.0f, -sc.N(2)));
		break;
	// TANGENT
	case DM_Tangent_Spherical: {
		float phi = std::acos(sc.Nx(2));
		float rho = std::atan2(sc.Nx(1), sc.Nx(0));

		RGBConverter::toSpec(spec, phi * PR_1_PI, rho * PR_1_PI * 0.5f, 0);
	} break;
	case DM_Tangent_Both:
		RGBConverter::toSpec(spec, std::abs(sc.Nx(0)),
							 std::abs(sc.Nx(1)),
							 std::abs(sc.Nx(2)));
		break;
	case DM_Tangent_Positive:
		RGBConverter::toSpec(spec, std::max(0.0f, sc.Nx(0)),
							 std::max(0.0f, sc.Nx(1)),
							 std::max(0.0f, sc.Nx(2)));
		break;
	case DM_Tangent_Negative:
		RGBConverter::toSpec(spec, std::max(0.0f, -sc.Nx(0)),
							 std::max(0.0f, -sc.Nx(1)),
							 std::max(0.0f, -sc.Nx(2)));
		break;
	// BINORMAL
	case DM_Binormal_Spherical: {
		float phi = std::acos(sc.Ny(2));
		float rho = std::atan2(sc.Ny(1), sc.Ny(0));

		RGBConverter::toSpec(spec, phi * PR_1_PI, rho * PR_1_PI * 0.5f, 0);
	} break;
	case DM_Binormal_Both:
		RGBConverter::toSpec(spec, std::abs(sc.Ny(0)),
							 std::abs(sc.Ny(1)),
							 std::abs(sc.Ny(2)));
		break;
	case DM_Binormal_Positive:
		RGBConverter::toSpec(spec, std::max(0.0f, sc.Ny(0)),
							 std::max(0.0f, sc.Ny(1)),
							 std::max(0.0f, sc.Ny(2)));
		break;
	case DM_Binormal_Negative:
		RGBConverter::toSpec(spec, std::max(0.0f, -sc.N(0)),
							 std::max(0.0f, -sc.Ny(1)),
							 std::max(0.0f, -sc.Ny(2)));
		break;
	//OTHER STUFF
	case DM_UVW:
		RGBConverter::toSpec(spec, sc.UVW(0), sc.UVW(1), sc.UVW(2));
		break;
	case DM_PDF: {
		float full_pdf = 0;
		for (uint32 i = 0; i < 32; ++i) {
			Eigen::Vector3f rnd = session.tile()->random().get3D();
			MaterialSample ms   = sc.Material->sample(sc, rnd, session);
			full_pdf += ms.PDF_S;
		}

		full_pdf /= 32;

		RGBConverter::toSpec(spec, full_pdf, full_pdf, full_pdf);
	} break;
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

		if (!sc.Material) {
			RGBConverter::toSpec(spec, 1, 0, 0);
			break;
		}

		if (sc.N.squaredNorm() - 1 > PR_EPSILON) {
			RGBConverter::toSpec(spec, 1, 1, 0);
			break;
		}

		Eigen::Vector3f rnd = session.tile()->random().get3D();
		MaterialSample ms   = sc.Material->sample(sc, rnd, session);

		if (ms.L.squaredNorm() - 1 > PR_EPSILON) {
			RGBConverter::toSpec(spec, 1, 0, 1);
			break;
		}

		if (ms.PDF_S > PR_EPSILON)
			RGBConverter::toSpec(spec, 0, 1, 0);
		else
			RGBConverter::toSpec(spec, 0, 0, 1);
	} break;
	case DM_Flag_Inside:
		if (sc.Flags & SCF_Inside)
			RGBConverter::toSpec(spec, 0, 1, 0);
		else
			RGBConverter::toSpec(spec, 1, 0, 0);
		break;
	case DM_Emission:
		break;
	case DM_Container_ID:
		RGBConverter::toSpec(spec, entity->containerID(), entity->containerID(), entity->containerID());
		break;
	}
}
} // namespace PR
