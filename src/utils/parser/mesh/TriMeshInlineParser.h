#pragma once

#include "PR_Config.h"

#include <string>

namespace DL {
class DataGroup;
}

namespace PR {
class TriMesh;

class SceneLoader;
class Environment;
class TriMeshInlineParser {
public:
	std::shared_ptr<PR::TriMesh> parse(Environment* env, const DL::DataGroup& group) const;
};
} // namespace PR
