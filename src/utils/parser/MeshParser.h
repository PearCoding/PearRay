#pragma once

#include "PR_Config.h"

#include <string>

namespace DL {
class DataGroup;
}

namespace PR {
class MeshContainer;
class MeshParser {
public:
	static std::shared_ptr<MeshContainer> parse(const DL::DataGroup& group);
};
} // namespace PR
