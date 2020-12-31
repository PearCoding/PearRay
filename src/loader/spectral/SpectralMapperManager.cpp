#include "SpectralMapperManager.h"
#include "Environment.h"
#include "SceneLoadContext.h"

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
	if (!settings.spectralMapperFactory)
		settings.spectralMapperFactory = createAndAdd("spd");

	return settings.spectralMapperFactory != nullptr;
}

} // namespace PR
