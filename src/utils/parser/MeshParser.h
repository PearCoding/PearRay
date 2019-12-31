#pragma once

#include "PR_Config.h"

#include <string>

namespace DL {
class DataGroup;
}

namespace PR {
class MeshBase;
class MeshParser {
public:
	static std::shared_ptr<MeshBase> parse(const DL::DataGroup& group);
};
} // namespace PR
