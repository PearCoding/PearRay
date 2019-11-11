#pragma once

#include "Plugin.h"
#include <boost/dll.hpp>
#include <map>

namespace PR {

class Registry;
class PR_LIB_UTILS PluginManager {
public:
	std::shared_ptr<IPlugin> load(const std::wstring& path, const Registry& reg,
								  bool useFallbacks = true);

	std::shared_ptr<IPlugin> get(const std::wstring& path) const;
	bool has(const std::wstring& path) const;

	void unload(const std::wstring& path);
	void unloadAll();

	std::string dumpInformation() const;

private:
	bool try_load(const std::wstring& path, const Registry& reg, bool useFallbacks);

	struct PluginLibPair {
		std::shared_ptr<IPlugin> Plugin;
		boost::dll::shared_library Library;

		~PluginLibPair() {
			Plugin.reset();
			Library.unload();
		}
	};
	std::map<std::wstring, PluginLibPair> mLibraries;
};
} // namespace PR