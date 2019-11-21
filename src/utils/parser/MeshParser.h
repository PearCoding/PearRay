#pragma once

#include "PR_Config.h"

#include <string>

namespace DL {
class DataGroup;
}

namespace PR {
class MeshContainer;

class SceneLoader;
class Environment;
class MeshParser {
public:
	static std::shared_ptr<PR::MeshContainer> parse(Environment* env, const DL::DataGroup& group);
};
} // namespace PR
