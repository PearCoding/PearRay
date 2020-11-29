#include "SamplerManager.h"
#include "Environment.h"
#include "SceneLoadContext.h"

namespace PR {
SamplerManager::SamplerManager()
{
}

SamplerManager::~SamplerManager()
{
}

bool SamplerManager::createDefaultsIfNecessary(Environment* env)
{
	constexpr uint64 DEF_AA_SC	 = 128;
	constexpr uint64 DEF_LENS_SC = 1;
	constexpr uint64 DEF_TIME_SC = 1;
	constexpr uint64 DEF_SPEC_SC = 1;

	auto createAndAdd = [&](const char* type, int sample_count) -> std::shared_ptr<ISamplerFactory> {
		auto fac = this->getFactory(type);

		if (!fac)
			return nullptr;

		SceneLoadContext ctx(env, {});
		ctx.parameters().addParameter("sample_count", Parameter::fromInt(sample_count));
		return fac->create(type, ctx);
	};

	auto& settings = env->renderSettings();

	// Add if necessary
	if (!settings.aaSamplerFactory) {
		PR_LOG(L_WARNING) << "No AA sampler selected. Using sobol sampler with sample count " << DEF_AA_SC << std::endl;
		settings.aaSamplerFactory = createAndAdd("sobol", DEF_AA_SC);
	}

	if (!settings.lensSamplerFactory) {
		//PR_LOG(L_WARNING) << "No lens sampler selected. Using halton sampler with sample count " << DEF_LENS_SC << std::endl;
		settings.lensSamplerFactory = createAndAdd("halton", DEF_LENS_SC);
	}

	if (!settings.timeSamplerFactory) {
		//PR_LOG(L_WARNING) << "No time sampler selected. Using multi jittered sampler with sample count " << DEF_TIME_SC << std::endl;
		settings.timeSamplerFactory = createAndAdd("mjitt", DEF_TIME_SC);
	}

	if (!settings.spectralSamplerFactory) {
		//PR_LOG(L_WARNING) << "No spectral sampler selected. Using random sampler with sample count " << DEF_SPEC_SC << std::endl;
		settings.spectralSamplerFactory = createAndAdd("random", DEF_SPEC_SC);
	}

	// Check again
	if (!settings.aaSamplerFactory)
		PR_LOG(L_ERROR) << "No default AA sampler could be found. No plugins available?" << std::endl;

	if (!settings.lensSamplerFactory)
		PR_LOG(L_ERROR) << "No default Lens sampler could be found. No plugins available?" << std::endl;

	if (!settings.timeSamplerFactory)
		PR_LOG(L_ERROR) << "No default Time sampler could be found. No plugins available?" << std::endl;

	if (!settings.spectralSamplerFactory)
		PR_LOG(L_ERROR) << "No default Spectral sampler could be found. No plugins available?" << std::endl;

	return settings.aaSamplerFactory && settings.lensSamplerFactory && settings.timeSamplerFactory && settings.spectralSamplerFactory;
}

} // namespace PR
