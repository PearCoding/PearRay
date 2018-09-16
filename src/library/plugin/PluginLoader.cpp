#include "PluginLoader.h"
#include "Logger.h"
#include "registry/Registry.h"
#include <boost/dll.hpp>

namespace PR {
constexpr const char* PR_PLUGIN_ENV_VAR	= "PR_PLUGIN_PATH";
constexpr char PR_PLUGIN_ENV_VAR_SEPERATOR = ':';

constexpr const char* PR_PLUGIN_REG_VAR	= "plugins/path";
constexpr char PR_PLUGIN_REG_VAR_SEPERATOR = ':';

IPlugin* PluginLoader::load(const std::string& path, const Registry& reg, bool useFallbacks)
{
	auto load = [](const boost::filesystem::path& p) -> boost::shared_ptr<PluginInterface> {
		try {
			PR_LOG(L_DEBUG) << "Trying to load plugin " << p << std::endl;

			return boost::dll::import<PluginInterface>(p,
													   PR_DOUBLEQUOTE(PR_PLUGIN_API_INTERFACE_NAME));
		} catch (const boost::system::system_error& e) {
			PR_LOG(L_DEBUG) << "Error loading plugin " << p << ": " << e.what() << std::endl;
			return nullptr;
		}
	};

	const boost::filesystem::path op	   = path;
	boost::shared_ptr<PluginInterface> ptr = load(op);

	// 1 Fallback -> Use registry entry for parent directory
	if (!ptr && !op.is_absolute() && useFallbacks) {
		std::string regPluginPath = reg.getByGroup<std::string>(RG_RENDERER, PR_PLUGIN_REG_VAR, "");
		if (!regPluginPath.empty()) {
			std::istringstream stream(regPluginPath);
			std::string current_path;
			while (std::getline(stream, current_path, PR_PLUGIN_REG_VAR_SEPERATOR)) {
				ptr = load(current_path / op);
				if (ptr)
					break;
			}
		}
	}

	// 2 Fallback -> Use environment variables
	if (!ptr && !op.is_absolute() && useFallbacks) {
		const std::string plugin_paths = std::getenv(PR_PLUGIN_ENV_VAR);
		if (!plugin_paths.empty()) {
			std::istringstream stream(plugin_paths);
			std::string current_path;
			while (std::getline(stream, current_path, PR_PLUGIN_ENV_VAR_SEPERATOR)) {
				ptr = load(current_path / op);
				if (ptr)
					break;
			}
		}
	}

	if (!ptr) {
		PR_LOG(L_ERROR) << "Could not load plugin " << path << std::endl;
		return nullptr;
	}

	if (ptr->APIVersion < PR_PLUGIN_API_VERSION) {
		PR_LOG(L_ERROR) << "Plugin " << path << " has API version less " << PR_PLUGIN_API_VERSION << std::endl;
		return nullptr;
	}

	if (ptr->APIVersion > PR_PLUGIN_API_VERSION) {
		PR_LOG(L_ERROR) << "Plugin " << path << " has API version greater " << PR_PLUGIN_API_VERSION << ". It is from the future :O" << std::endl;
		return nullptr;
	}

	if (!ptr->InitFunction) {
		PR_LOG(L_ERROR) << "Plugin " << path << " has no valid init function!" << std::endl;
		return nullptr;
	}

	return ptr->InitFunction();
} // namespace PR
} // namespace PR
