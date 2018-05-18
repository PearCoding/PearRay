#include "DebugIntegrator.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "ray/Ray.h"

#include "scene/Scene.h"

#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderTile.h"

#include "shader/ShaderClosure.h"
#include "spectral/RGBConverter.h"

namespace PR {
// Registry entries:
static const char* RE_DEBUG_MODE = "visualizer/mode";

DebugIntegrator::DebugIntegrator(RenderContext* renderer)
	: OnePassIntegrator(renderer)
	, mDebugMode(DM_NORMAL_BOTH)
{
}

void DebugIntegrator::init()
{
	OnePassIntegrator::init();

	mDebugMode = renderer()->registry()->getByGroup<DebugMode>(RG_INTEGRATOR,
															   RE_DEBUG_MODE,
															   DM_NORMAL_BOTH);

	// Only approximative
	mMaxDepth = renderer()->scene()->boundingBox().diameter();
}

void DebugIntegrator::onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session)
{
	RenderEntity* entity;

	Ray out = in;
	out.setFlags(in.flags() | RF_Debug);

	spec.clear();

	if (mDebugMode == DM_BOUNDING_BOX) {
		Eigen::Vector3f p;
		entity = renderer()->shootForBoundingBox(out, p, session);

		if(entity) {
			const float depth = (p-out.origin()).norm();
			spec.fill(depth/mMaxDepth);
			//Ray next = out.next(p, out.direction());
			//onPixel()
		}
	} else {
		if (mDebugMode == DM_EMISSION)
			entity = renderer()->shootWithEmission(spec, out, sc, session);
		else
			entity = renderer()->shoot(out, sc, session);

		if (!entity || (mDebugMode != DM_VALIDITY && !sc.Material))
			return;

		switch (mDebugMode) {
		default:
		case DM_DEPTH: {
			float depth = (in.origin() - sc.P).norm()/mMaxDepth;
			RGBConverter::toSpec(spec, depth, depth, depth);
		} break;
		// NORMAL
		case DM_NORMAL_SPHERICAL: {
			float phi = std::acos(sc.N(2));
			float rho = std::atan2(sc.N(1), sc.N(0));

			RGBConverter::toSpec(spec, phi * PR_1_PI, rho * PR_1_PI * 0.5f, 0);
		} break;
		case DM_NORMAL_BOTH:
			RGBConverter::toSpec(spec, std::abs(sc.N(0)),
								 std::abs(sc.N(1)),
								 std::abs(sc.N(2)));
			break;
		case DM_NORMAL_POSITIVE:
			RGBConverter::toSpec(spec, std::max(0.0f, sc.N(0)),
								 std::max(0.0f, sc.N(1)),
								 std::max(0.0f, sc.N(2)));
			break;
		case DM_NORMAL_NEGATIVE:
			RGBConverter::toSpec(spec, std::max(0.0f, -sc.N(0)),
								 std::max(0.0f, -sc.N(1)),
								 std::max(0.0f, -sc.N(2)));
			break;
		// TANGENT
		case DM_TANGENT_SPHERICAL: {
			float phi = std::acos(sc.Nx(2));
			float rho = std::atan2(sc.Nx(1), sc.Nx(0));

			RGBConverter::toSpec(spec, phi * PR_1_PI, rho * PR_1_PI * 0.5f, 0);
		} break;
		case DM_TANGENT_BOTH:
			RGBConverter::toSpec(spec, std::abs(sc.Nx(0)),
								 std::abs(sc.Nx(1)),
								 std::abs(sc.Nx(2)));
			break;
		case DM_TANGENT_POSITIVE:
			RGBConverter::toSpec(spec, std::max(0.0f, sc.Nx(0)),
								 std::max(0.0f, sc.Nx(1)),
								 std::max(0.0f, sc.Nx(2)));
			break;
		case DM_TANGENT_NEGATIVE:
			RGBConverter::toSpec(spec, std::max(0.0f, -sc.Nx(0)),
								 std::max(0.0f, -sc.Nx(1)),
								 std::max(0.0f, -sc.Nx(2)));
			break;
		// BINORMAL
		case DM_BINORMAL_SPHERICAL: {
			float phi = std::acos(sc.Ny(2));
			float rho = std::atan2(sc.Ny(1), sc.Ny(0));

			RGBConverter::toSpec(spec, phi * PR_1_PI, rho * PR_1_PI * 0.5f, 0);
		} break;
		case DM_BINORMAL_BOTH:
			RGBConverter::toSpec(spec, std::abs(sc.Ny(0)),
								 std::abs(sc.Ny(1)),
								 std::abs(sc.Ny(2)));
			break;
		case DM_BINORMAL_POSITIVE:
			RGBConverter::toSpec(spec, std::max(0.0f, sc.Ny(0)),
								 std::max(0.0f, sc.Ny(1)),
								 std::max(0.0f, sc.Ny(2)));
			break;
		case DM_BINORMAL_NEGATIVE:
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
		case DM_VALIDITY: {
			constexpr float eps = 0.0001f;
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

			if (sc.N.squaredNorm() - 1.0f > eps) {
				RGBConverter::toSpec(spec, 1, 1, 0);
				break;
			}

			Eigen::Vector3f rnd = session.tile()->random().get3D();
			MaterialSample ms   = sc.Material->sample(sc, rnd, session);

			if (ms.L.squaredNorm() - 1.0f > eps) {
				RGBConverter::toSpec(spec, 1, 0, 1);
				break;
			}

			if (ms.PDF_S > PR_EPSILON)
				RGBConverter::toSpec(spec, 0, 1, 0);
			else
				RGBConverter::toSpec(spec, 0, 0, 1);
		} break;
		case DM_FLAG_INSIDE:
			if (sc.Flags & SCF_Inside)
				RGBConverter::toSpec(spec, 0, 1, 0);
			else
				RGBConverter::toSpec(spec, 1, 0, 0);
			break;
		case DM_EMISSION:
			break;
		case DM_CONTAINER_ID:
			RGBConverter::toSpec(spec, entity->containerID(), entity->containerID(), entity->containerID());
			break;
		}
	}
}
} // namespace PR
