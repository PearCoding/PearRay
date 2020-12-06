#include "PluginManager.h"
#include "Logger.h"

#include <regex>

namespace PR {
constexpr static const char* PR_PLUGIN_ENV_VAR	  = "PR_PLUGIN_PATH";
constexpr static char PR_PLUGIN_ENV_VAR_SEPERATOR = ':';

PluginManager::PluginManager()
{
}

void PluginManager::initPlugins(const std::filesystem::path& workingDir, const std::filesystem::path& pluginDir)
{
	loadEmbeddedPlugins();
	if (!pluginDir.empty())
		loadFromDirectory(pluginDir);
	if (!workingDir.empty() && workingDir != pluginDir)
		loadFromDirectory(workingDir);

	char const* plugin_paths = std::getenv(PR_PLUGIN_ENV_VAR);
	if (plugin_paths) {
		std::istringstream stream(plugin_paths);
		std::string current_path;
		while (std::getline(stream, current_path, PR_PLUGIN_ENV_VAR_SEPERATOR))
			loadFromDirectory(current_path);
	}
}

void PluginManager::loadFromDirectory(const std::filesystem::path& path)
{
	try {
#ifdef PR_DEBUG
		static const std::wregex e(L"(lib)?pr_pl_([\\w_]+)_d");
#else
		static const std::wregex e(L"(lib)?pr_pl_([\\w_]+)");
#endif

		// Load dlls
		for (auto& entry : std::filesystem::directory_iterator(path)) {
			if (!std::filesystem::is_regular_file(entry))
				continue;

			const std::wstring filename = entry.path().stem().generic_wstring();
			const std::wstring ext		= entry.path().extension().generic_wstring();

			if (ext != L".so" && ext != L".dll")
				continue;

			std::wsmatch what;
			if (std::regex_match(filename, what, e)) {
#ifndef PR_DEBUG
				// Ignore debug builds
				if (filename.substr(filename.size() - 2, 2) == L"_d")
					continue;
#endif

				load(entry.path());
			}
		}
	} catch (const std::exception& e) {
		PR_LOG(L_ERROR) << "Could not load external plugins: " << e.what() << std::endl;
	}
}

bool PluginManager::tryLoad(const std::filesystem::path& path)
{
	auto load = [](const std::filesystem::path& p) -> SharedLibrary {
		try {
			PR_LOG(L_DEBUG) << "Trying to load plugin " << p << std::endl;
			return SharedLibrary(p.generic_wstring());
		} catch (const std::exception& e) {
			PR_LOG(L_DEBUG) << "Error loading plugin " << p << ": " << e.what() << std::endl;
			return SharedLibrary();
		}
	};

	auto lib = load(path);

	if (!lib) {
		PR_LOG(L_ERROR) << "Could not load plugin " << path << std::endl;
		return false;
	}

	PluginInterface* ptr = reinterpret_cast<PluginInterface*>(lib.symbol(PR_DOUBLEQUOTE(PR_PLUGIN_API_INTERFACE_NAME_CORE)));
	if (!ptr) {
		PR_LOG(L_ERROR) << "Could not find plugin interface for " << path << std::endl;
		return false;
	}

	if (!loadInterface(path.generic_string(), ptr))
		return false;

	auto plugin = std::shared_ptr<IPlugin>(ptr->InitFunction());
	mLibraries.emplace(
		std::make_pair(ptr->PluginName,
					   PluginLibPair{ plugin, path, lib }));

	return true;
}

std::shared_ptr<IPlugin> PluginManager::load(const std::filesystem::path& path)
{
#ifdef PR_DEBUG
	std::filesystem::path p = path;
	if (!tryLoad(path)) {
		std::wstring rel_name = p.stem().generic_wstring();
		size_t pos			  = rel_name.find_last_of(L"_d");
		if (pos == std::wstring::npos)
			return nullptr;

		rel_name.erase(pos, 2);

		std::filesystem::path release_path = p.parent_path() / (rel_name + p.extension().generic_wstring());
		if (!tryLoad(release_path.generic_wstring())) {
			return nullptr;
		} else {
			return getFromPath(path);
		}
	} else {
		return getFromPath(path);
	}
#else
	if (!tryLoad(path))
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

std::shared_ptr<IPlugin> PluginManager::getFromPath(const std::filesystem::path& path) const
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
							   PluginLibPair{ plugin, L"", SharedLibrary() }));
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

	for (const auto& p : mLibraries)
		list.push_back(p.second.Plugin);
	return list;
}
} // namespace PR
