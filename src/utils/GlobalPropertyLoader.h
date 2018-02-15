#pragma once

#include "renderer/RenderSettings.h"

#include <map>
#include <string>

namespace DL {
class DataGroup;
}

namespace PR {
class Environment;
class PR_LIB_UTILS GlobalPropertyLoader {
	PR_CLASS_NON_CONSTRUCTABLE(GlobalPropertyLoader);

public:
	static RenderSettings loadGlobalProperties(const DL::DataGroup& group);

private:
	static void handleProperty(const DL::DataGroup& group, RenderSettings& settings);
};
} // namespace PR
