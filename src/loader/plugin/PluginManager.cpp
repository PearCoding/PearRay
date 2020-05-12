#include "PluginManager.h"
#include "Logger.h"

namespace PR {
constexpr static const char* PR_PLUGIN_ENV_VAR	= "PR_PLUGIN_PATH";
constexpr static char PR_PLUGIN_ENV_VAR_SEPERATOR = ':';

PluginManager::PluginManager(const std::wstring& pluginDir)
	: mPluginDir(pluginDir)
{
}

bool PluginManager::tryLoad(const std::wstring& path, bool useFallbacks)
{
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

	// 1 Fallback -> Use plugin directory for parent directory
	if (!lib && !op.is_absolute() && useFallbacks && !mPluginDir.empty()) {
		lib = load(mPluginDir / op);
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
		PR_LOG(L_ERROR) << "Could not load plugin " << op << std::endl;
		return false;
	}

	PluginInterface* ptr = nullptr;
	try {
		ptr = &lib.get<PluginInterface>(PR_DOUBLEQUOTE(PR_PLUGIN_API_INTERFACE_NAME_CORE));
	} catch (const boost::system::system_error& e) {
		PR_LOG(L_DEBUG) << "Internal error: " << e.what() << std::endl;
	}

	if (!loadInterface(op.generic_string(), ptr))
		return false;

	auto plugin = std::shared_ptr<IPlugin>(ptr->InitFunction());
	mLibraries.emplace(
		std::make_pair(ptr->PluginName,
					   PluginLibPair{ plugin, path, lib }));

	return true;
}

std::shared_ptr<IPlugin> PluginManager::load(const std::wstring& path, bool useFallbacks)
{
#ifdef PR_DEBUG
	boost::filesystem::path p = path;
	if (!tryLoad(path, useFallbacks)) {
		std::wstring rel_name = p.stem().generic_wstring();
		size_t pos			  = rel_name.find_last_of(L"_d");
		if (pos == std::wstring::npos)
			return nullptr;

		rel_name.erase(pos, 2);

		boost::filesystem::path release_path = p.parent_path() / (rel_name + p.extension().generic_wstring());
		if (!tryLoad(release_path.generic_wstring(), useFallbacks)) {
			return nullptr;
		} else {
			return getFromPath(path);
		}
	} else {
		return getFromPath(path);
	}
#else
	if (!tryLoad(path, useFallbacks))
		return nullptr;
	else
		return getFromPath(path);
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

std::shared_ptr<IPlugin> PluginManager::get(const std::string& name) const
{
	if (has(name))
		return mLibraries.at(name).Plugin;
	else
		return nullptr;
}

std::shared_ptr<IPlugin> PluginManager::getFromPath(const std::wstring& path) const
{
	for (const auto& p : mLibraries) {
		if (p.second.Path == path)
			return p.second.Plugin;
	}

	return nullptr;
}

bool PluginManager::has(const std::string& name) const
{
	return mLibraries.count(name);
}

#include "_pr_embedded_plugins.h"

void PluginManager::loadEmbeddedPlugins()
{
	for (int i = 0; __embedded_plugins[i].Name; ++i) {
		auto ptr = __embedded_plugins[i].Interface;
		PR_LOG(L_DEBUG) << "Loading embedded plugin " << __embedded_plugins[i].Name << std::endl;
		if (loadInterface(__embedded_plugins[i].Name, ptr)) {
			auto plugin = std::shared_ptr<IPlugin>(ptr->InitFunction());
			mLibraries.emplace(
				std::make_pair(ptr->PluginName,
							   PluginLibPair{ plugin, L"", boost::dll::shared_library() }));
		}
	}
}

bool PluginManager::loadInterface(const std::string& name, PluginInterface* ptr)
{
	if (!ptr) {
		PR_LOG(L_ERROR) << "Could not get plugin interface for " << name << std::endl;
		return false;
	}

	if (has(ptr->PluginName)) {
		PR_LOG(L_INFO) << "Ignoring plugin " << name << ". Already loaded" << std::endl;
		return false;
	}

	if (ptr->APIVersion < PR_PLUGIN_API_VERSION) {
		PR_LOG(L_ERROR) << "Plugin " << name << " has API version less " << PR_PLUGIN_API_VERSION << std::endl;
		return false;
	}

	if (ptr->APIVersion > PR_PLUGIN_API_VERSION) {
		PR_LOG(L_ERROR) << "Plugin " << name << " has API version greater " << PR_PLUGIN_API_VERSION << ". It is from the future :O" << std::endl;
		return false;
	}

	if (!ptr->InitFunction) {
		PR_LOG(L_ERROR) << "Plugin " << name << " has no valid init function!" << std::endl;
		return false;
	}

	return true;
}

std::vector<std::shared_ptr<IPlugin>> PluginManager::plugins() const
{
	std::vector<std::shared_ptr<IPlugin>> list;
	list.reserve(mLibraries.size());

	for (const auto& p : mLibraries) {
		list.push_back(p.second.Plugin);
	}
	return list;
}
} // namespace PR
