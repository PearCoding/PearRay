#pragma once

#include "Profiler.h"
#include "SceneLoadContext.h"
#include "infinitelight/IInfiniteLight.h"
#include "light/LightSampler.h"
#include "path/LightPath.h"
#include "renderer/RenderTileSession.h"
#include "shader/ShadingGroup.h"

#include <optional>

namespace PR {
class IntegratorUtils {
public:
	static inline void handleBackgroundGroup(RenderTileSession& session, const ShadingGroup& sg)
	{
		PR_PROFILE_THIS;
		const LightPath cb = LightPath::createCB();
		session.tile()->statistics().add(RST_CameraDepthCount, sg.size());
		session.tile()->statistics().add(RST_BackgroundHitCount, sg.size());

		bool illuminated		= false;
		const auto lightSampler = session.context()->lightSampler();

		for (auto light : lightSampler->infiniteLights()) {
			if (light->hasDeltaDistribution())
				continue;

			illuminated = true;
			for (size_t i = 0; i < sg.size(); ++i) {
				Ray ray;
				sg.extractRay(i, ray);

				InfiniteLightEvalInput lin;
				lin.WavelengthNM   = ray.WavelengthNM;
				lin.Direction	   = ray.Direction;
				lin.IterationDepth = ray.IterationDepth;
				InfiniteLightEvalOutput lout;
				light->asInfiniteLight()->eval(lin, lout, session);

				session.pushSpectralFragment(1, SpectralBlob::Ones(), lout.Radiance, ray, cb);
			}
		}

		if (!illuminated) { // If no inf. lights are available make sure at least zero is splatted
			for (size_t i = 0; i < sg.size(); ++i) {
				Ray ray;
				sg.extractRay(i, ray);
				session.pushSpectralFragment(1, SpectralBlob::Ones(), SpectralBlob::Zero(), ray, cb);
			}
		}
	}

	template <typename Func>
	static inline bool handleBackground(RenderTileSession& session, const Ray& ray, const Func& func)
	{
		session.tile()->statistics().add(RST_BackgroundHitCount);

		bool illuminated		= false;
		const auto lightSampler = session.context()->lightSampler();

		for (auto light : lightSampler->infiniteLights()) {
			if (light->hasDeltaDistribution())
				continue;

			illuminated = true;

			InfiniteLightEvalInput lin;
			lin.WavelengthNM   = ray.WavelengthNM;
			lin.Direction	   = ray.Direction;
			lin.IterationDepth = ray.IterationDepth;
			InfiniteLightEvalOutput lout;
			light->asInfiniteLight()->eval(lin, lout, session);

			func(lout);
		}

		return illuminated;
	}
};

} // namespace PR