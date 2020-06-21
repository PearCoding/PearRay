#pragma once

#include "Plugin.h"
#include "arch/SharedLibrary.h"

#include <unordered_map>
#include <unordered_set>

namespace PR {

class PR_LIB_LOADER PluginManager {
public:
	explicit PluginManager(const std::wstring& pluginDir);

	void loadEmbeddedPlugins();
	std::shared_ptr<IPlugin> load(const std::wstring& path,
								  bool useFallbacks = true);

	std::shared_ptr<IPlugin> get(const std::string& name) const;
	std::shared_ptr<IPlugin> getFromPath(const std::wstring& path) const;
	bool has(const std::string& name) const;

	void unload(const std::string& name);
	void unloadAll();

	std::string dumpInformation() const;

	std::vector<std::shared_ptr<IPlugin>> plugins() const;

private:
	bool tryLoad(const std::wstring& path, bool useFallbacks);
	bool loadInterface(const std::string& name, struct PluginInterface* interface);

	struct PluginLibPair {
		std::shared_ptr<IPlugin> Plugin;
		std::wstring Path;
		SharedLibrary Library;

		~PluginLibPair()
		{
			Plugin.reset();
			if (!Path.empty())
				Library.unload();
		}
	};
	std::unordered_map<std::string, PluginLibPair> mLibraries;
	std::wstring mPluginDir;
};
} // namespace PR