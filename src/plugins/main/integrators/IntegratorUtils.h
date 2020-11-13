#pragma once

#include "Profiler.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "path/LightPath.h"

#include <optional>

namespace PR {
class IntegratorUtils {
public:
	static inline void handleBackgroundGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;
		LightPath cb = LightPath::createCB();
		session.tile()->statistics().addCameraDepthCount(sg.size());
		session.tile()->statistics().addBackgroundHitCount(sg.size());

		if (!session.tile()->context()->scene()->nonDeltaInfiniteLights().empty()) {
			for (auto light : session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
				for (size_t i = 0; i < sg.size(); ++i) {
					InfiniteLightEvalInput in;
					Ray ray;
					sg.extractRay(i, ray);
					in.WavelengthNM	  = ray.WavelengthNM;
					in.Direction	  = ray.Direction;
					in.IterationDepth = ray.IterationDepth;
					InfiniteLightEvalOutput out;
					light->eval(in, out, session);

					session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), out.Radiance, ray, cb);
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

	template <typename Func>
	static inline void handleBackground(RenderTileSession& session, const Ray& ray, const Func& func)
	{
		session.tile()->statistics().addBackgroundHitCount();
		for (auto light : session.tile()->context()->scene()->nonDeltaInfiniteLights()) {
			InfiniteLightEvalInput lin;
			lin.WavelengthNM   = ray.WavelengthNM;
			lin.Direction	   = ray.Direction;
			lin.IterationDepth = ray.IterationDepth;
			InfiniteLightEvalOutput lout;
			light->eval(lin, lout, session);

			if (PR_UNLIKELY(lout.Direction_PDF_S <= PR_EPSILON))
				continue;

			func(lout);
		}
	}
};

} // namespace PR