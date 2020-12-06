#pragma once

#include "Plugin.h"
#include "arch/SharedLibrary.h"

#include <unordered_map>
#include <unordered_set>
#include <filesystem>

namespace PR {

class PR_LIB_LOADER PluginManager {
public:
	explicit PluginManager();

	void initPlugins(const std::filesystem::path& workingDir, const std::filesystem::path& pluginDir);

	void loadEmbeddedPlugins();
	void loadFromDirectory(const std::filesystem::path& path);

	std::shared_ptr<IPlugin> load(const std::filesystem::path& path);

	std::shared_ptr<IPlugin> get(const std::string& name) const;
	std::shared_ptr<IPlugin> getFromPath(const std::filesystem::path& path) const;
	bool has(const std::string& name) const;

	void unload(const std::string& name);
	void unloadAll();

	std::string dumpInformation() const;

	std::vector<std::shared_ptr<IPlugin>> plugins() const;

private:
	bool tryLoad(const std::filesystem::path& path);
	bool loadInterface(const std::string& name, struct PluginInterface* interface);

	struct PluginLibPair {
		std::shared_ptr<IPlugin> Plugin;
		std::filesystem::path Path;
		SharedLibrary Library;

		~PluginLibPair()
		{
			Plugin.reset();
			if (!Path.empty())
				Library.unload();
		}
	};
	std::unordered_map<std::string, PluginLibPair> mLibraries;
};
} // namespace PR