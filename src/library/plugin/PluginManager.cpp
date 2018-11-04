#include "PluginManager.h"
#include "Logger.h"
#include "registry/Registry.h"

namespace PR {
constexpr static const char* PR_PLUGIN_ENV_VAR	= "PR_PLUGIN_PATH";
constexpr static char PR_PLUGIN_ENV_VAR_SEPERATOR = ':';

constexpr static const char* PR_PLUGIN_REG_VAR	= "plugins/path";
constexpr static char PR_PLUGIN_REG_VAR_SEPERATOR = ':';

bool PluginManager::try_load(const std::string& path, const Registry& reg, bool useFallbacks)
{
	if (has(path)) {
		return true;
	}

	auto load = [](const boost::filesystem::path& p) -> boost::dll::shared_library {
		try {
			PR_LOG(L_DEBUG) << "Trying to load plugin " << p << std::endl;
			return boost::dll::shared_library(p,
											  boost::dll::load_mode::append_decorations);
		} catch (const boost::system::system_error& e) {
			PR_LOG(L_DEBUG) << "Error loading plugin " << p << ": " << e.what() << std::endl;
			return boost::dll::shared_library();
		}
	};

	const boost::filesystem::path op = path;
	auto lib						 = load(op);

	// 1 Fallback -> Use registry entry for parent directory
	if (!lib && !op.is_absolute() && useFallbacks) {
		std::string regPluginPath = reg.getByGroup<std::string>(RG_RENDERER, PR_PLUGIN_REG_VAR, "");
		if (!regPluginPath.empty()) {
			std::istringstream stream(regPluginPath);
			std::string current_path;
			while (std::getline(stream, current_path, PR_PLUGIN_REG_VAR_SEPERATOR)) {
				lib = load(current_path / op);
				if (lib)
					break;
			}
		}
	}

	// 2 Fallback -> Use environment variables
	if (!lib && !op.is_absolute() && useFallbacks) {
		char const* plugin_paths = std::getenv(PR_PLUGIN_ENV_VAR);
		if (plugin_paths) {
			std::istringstream stream(plugin_paths);
			std::string current_path;
			while (std::getline(stream, current_path, PR_PLUGIN_ENV_VAR_SEPERATOR)) {
				lib = load(current_path / op);
				if (lib)
					break;
			}
		}
	}

	if (!lib) {
		PR_LOG(L_ERROR) << "Could not load plugin " << path << std::endl;
		return false;
	}

	PluginInterface* ptr = nullptr;
	try {
		ptr = &lib.get<PluginInterface>(PR_DOUBLEQUOTE(PR_PLUGIN_API_INTERFACE_NAME));
	} catch (const boost::system::system_error& e) {
		PR_LOG(L_DEBUG) << "Internal error: " << e.what() << std::endl;
	}

	if (!ptr) {
		PR_LOG(L_ERROR) << "Could not get plugin interface for " << path << std::endl;
		return false;
	}

	if (ptr->APIVersion < PR_PLUGIN_API_VERSION) {
		PR_LOG(L_ERROR) << "Plugin " << path << " has API version less " << PR_PLUGIN_API_VERSION << std::endl;
		return false;
	}

	if (ptr->APIVersion > PR_PLUGIN_API_VERSION) {
		PR_LOG(L_ERROR) << "Plugin " << path << " has API version greater " << PR_PLUGIN_API_VERSION << ". It is from the future :O" << std::endl;
		return false;
	}

	if (!ptr->InitFunction) {
		PR_LOG(L_ERROR) << "Plugin " << path << " has no valid init function!" << std::endl;
		return false;
	}

	mLibraries.emplace(
		std::make_pair(path,
					   PluginLibPair{ std::shared_ptr<IPlugin>(ptr->InitFunction()), lib }));
	return true;
}

std::shared_ptr<IPlugin> PluginManager::load(const std::string& path, const Registry& reg, bool useFallbacks)
{
#ifdef PR_DEBUG
	if (!try_load(path + "_d", reg, useFallbacks)) {
		if (!try_load(path, reg, useFallbacks)) {
			return nullptr;
		}
	}
	return get(path);
#else
	if (!try_load(path, reg, useFallbacks))
		return nullptr;
	else
		return get(path);
#endif
}

void PluginManager::unload(const std::string& path)
{
	mLibraries.erase(path);
}

void PluginManager::unloadAll()
{
	mLibraries.clear();
}

std::string PluginManager::dumpInformation() const
{
	return "";
}

std::shared_ptr<IPlugin> PluginManager::get(const std::string& path) const
{
	if (has(path))
		return mLibraries.at(path).Plugin;
	else
		return nullptr;
}

bool PluginManager::has(const std::string& path) const
{
	return mLibraries.count(path);
}
} // namespace PR
