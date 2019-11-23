#pragma once

#include "Plugin.h"
#include <boost/dll.hpp>
#include <unordered_map>
#include <unordered_set>

namespace PR {

class Registry;
class PR_LIB_UTILS PluginManager {
public:
	void loadEmbeddedPlugins();
	std::shared_ptr<IPlugin> load(const std::wstring& path, const Registry& reg,
								  bool useFallbacks = true);

	std::shared_ptr<IPlugin> get(const std::wstring& path) const;
	bool has(const std::wstring& path) const;

	void unload(const std::wstring& path);
	void unloadAll();

	std::string dumpInformation() const;

private:
	bool tryLoad(const std::wstring& path, const Registry& reg, bool useFallbacks);
	bool loadInterface(const std::string& name, class PluginInterface* interface);

	struct PluginLibPair {
		std::shared_ptr<IPlugin> Plugin;
		boost::dll::shared_library Library;

		~PluginLibPair() {
			Plugin.reset();
			Library.unload();
		}
	};
	std::unordered_map<std::wstring, PluginLibPair> mLibraries;
	std::unordered_set<std::string> mEmbeddedPlugins;
};
} // namespace PR