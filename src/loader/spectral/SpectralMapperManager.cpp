#include "SpectralMapperManager.h"
#include "Environment.h"
#include "SceneLoadContext.h"
#include "spectral/CIE.h"

namespace PR {
SpectralMapperManager::SpectralMapperManager()
{
}

SpectralMapperManager::~SpectralMapperManager()
{
}

bool SpectralMapperManager::createDefaultsIfNecessary(Environment* env)
{
	const auto createAndAdd = [&](const char* type) -> std::shared_ptr<ISpectralMapperFactory> {
		auto fac = this->getFactory(type);

		if (!fac)
			return nullptr;

		SceneLoadContext ctx(env, {});
		return fac->create(type, ctx);
	};

	auto& settings = env->renderSettings();

	// Add if necessary
	if (!settings.spectralMapperFactory) {
		if (settings.spectralStart >= PR_VISIBLE_WAVELENGTH_START && settings.spectralEnd <= PR_VISIBLE_WAVELENGTH_END) {
			settings.spectralMapperFactory = createAndAdd("visible");
		} else if (settings.spectralStart >= PR_CIE_WAVELENGTH_START && settings.spectralEnd <= PR_CIE_WAVELENGTH_END) {
			settings.spectralMapperFactory = createAndAdd("cie");
		} else {
			PR_LOG(L_WARNING) << "No spectral mapper selected for unusual spectral domain. Using random spectral mapper" << std::endl;
			settings.spectralMapperFactory = createAndAdd("random");
		}
	}

	return settings.spectralMapperFactory != nullptr;
}

} // namespace PR
