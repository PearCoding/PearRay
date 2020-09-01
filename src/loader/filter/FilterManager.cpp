#include "FilterManager.h"
#include "Environment.h"
#include "SceneLoadContext.h"

namespace PR {
FilterManager::FilterManager()
{
}

FilterManager::~FilterManager()
{
}

bool FilterManager::createDefaultsIfNecessary(Environment* env)
{
	constexpr uint64 DEF_R = 1;

	auto createAndAdd = [&](const char* type, int radius) -> std::shared_ptr<IFilterFactory> {
		auto fac = this->getFactory(type);
		if (!fac)
			fac = this->mFactories.begin()->second;

		if (!fac)
			return nullptr;

		const uint32 id = this->nextID();

		SceneLoadContext ctx(env);
		ctx.parameters().addParameter("radius", Parameter::fromInt(radius));
		auto obj = fac->create(id, type, ctx);
		if (obj)
			this->addObject(obj);
		return obj;
	};

	auto& settings = env->renderSettings();

	// Add if necessary
	if (!settings.pixelFilterFactory) {
		//PR_LOG(L_WARNING) << "No pixel filter selected. Using mitchell filter with radius " << DEF_R << std::endl;
		settings.pixelFilterFactory = createAndAdd("mitchell", DEF_R);
	}

	// Check again
	if (!settings.pixelFilterFactory)
		PR_LOG(L_ERROR) << "No default pixel filter could be found. No plugins available?" << std::endl;

	return settings.pixelFilterFactory != nullptr;
}

} // namespace PR
