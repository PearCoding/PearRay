#pragma once

#include "Plugin.h"

namespace PR {

class Registry;
class PR_LIB PluginLoader {
public:
	static IPlugin* load(const std::string& path, const Registry& reg, bool useFallbacks = true);
};
} // namespace PR