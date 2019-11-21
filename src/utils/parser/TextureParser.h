#pragma once

#include "PR_Config.h"

#include <string>

namespace DL {
class DataGroup;
}

namespace PR {
class Environment;
class SceneLoader;
class PR_LIB_UTILS TextureParser {
public:
	void parse(Environment* env,
			   const std::string& name, const DL::DataGroup& group) const;
};
} // namespace PR
