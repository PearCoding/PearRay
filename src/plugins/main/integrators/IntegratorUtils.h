#pragma once

#include "Profiler.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "path/LightPath.h"

#include "IntegratorDefaults.h"

#include <optional>

namespace PR {
class IntegratorUtils {
public:
	struct Sample {
		SpectralBlob Weight;
		float PDF_S;
		Vector3f Direction;
	};

	static inline std::optional<Sample> sampleInfiniteLight(RenderTileSession& session, const IntersectionPoint& spt, IInfiniteLight* infLight)
	{
		PR_PROFILE_THIS;

		// Sample light
		InfiniteLightSampleInput inL;
		inL.WavelengthNM = spt.Ray.WavelengthNM;
		inL.RND			 = session.tile()->random().get2D();
		InfiniteLightSampleOutput outL;
		infLight->sample(inL, outL, session);

		// An unlikely event, but a zero pdf has to be catched before blowing up other parts
		if (PR_UNLIKELY(outL.PDF_S <= PR_EPSILON))
			return {};

		if (infLight->hasDeltaDistribution())
			outL.PDF_S = 1;

		// Trace shadow ray
		const Ray shadow		= spt.Ray.next(spt.P, outL.Outgoing, spt.Surface.N, RF_Shadow, SHADOW_RAY_MIN, SHADOW_RAY_MAX);
		const bool hitSomething = session.traceOcclusionRay(shadow);

		SpectralBlob evalW;
		if (hitSomething) // If we hit something before the light/background, the light path is occluded
			evalW = SpectralBlob::Zero();
		else
			evalW = outL.Radiance;

		return std::make_optional(Sample{ evalW, outL.PDF_S, outL.Outgoing });
	}

	static inline void handleBackgroundGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;
		LightPath cb = LightPath::createCB();
		session.tile()->statistics().addDepthCount(sg.size());
		session.tile()->statistics().addBackgroundHitCount(sg.size());

		if (!session.tile()->context()->scene()->nonDeltaInfiniteLights().empty()) {
			for (auto light : session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
				for (size_t i = 0; i < sg.size(); ++i) {
					InfiniteLightEvalInput in;
					sg.extractRay(i, in.Ray);
					in.Point = nullptr;
					InfiniteLightEvalOutput out;
					light->eval(in, out, session);

					session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), out.Radiance, in.Ray, cb);
				}
			}
		} else { // If no inf. lights are available make sure at least zero is splatted
			for (size_t i = 0; i < sg.size(); ++i) {
				Ray ray;
				sg.extractRay(i, ray);
				session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), SpectralBlob::Zero(), ray, cb);
			}
		}
	}

	static inline void handleBackground(RenderTileSession& session, LightPath& path,
										const SpectralBlob& weight, const Ray& ray)
	{
		session.tile()->statistics().addBackgroundHitCount();
		path.addToken(LightPathToken::Background());
		for (auto light : session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
			InfiniteLightEvalInput lin;
			lin.Point = nullptr; // TODO
			lin.Ray	  = ray;
			InfiniteLightEvalOutput lout;
			light->eval(lin, lout, session);

			if (PR_UNLIKELY(lout.PDF_S <= PR_EPSILON))
				continue;

			session.pushSpectralFragment(SpectralBlob::Ones(), weight, lout.Radiance, ray, path);
		}
		path.popToken();
	}
};

} // namespace PR