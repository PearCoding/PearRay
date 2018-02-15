#pragma once

#include "PR_Config.h"

#include <string>

namespace DL {
class DataGroup;
}

namespace PR {
class Material;

class Environment;
class SceneLoader;
class PR_LIB_UTILS_INLINE IMaterialParser {
public:
	virtual std::shared_ptr<Material> parse(Environment* env,
											const std::string& obj, const DL::DataGroup& group) const = 0;
};
} // namespace PR
