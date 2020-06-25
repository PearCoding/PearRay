#pragma once

#include "shader/Node.h"

namespace DL {
class Data;
} // namespace DL

namespace PR {
class Environment;
class SocketParser {
public:
	static std::shared_ptr<FloatSpectralNode> getSpectralOutput(
		Environment* env, const DL::Data& data, bool allowScalar = false);
	static std::shared_ptr<FloatScalarNode> getScalarOutput(
		Environment* env, const DL::Data& data);
	static std::shared_ptr<FloatVectorNode> getVectorOutput(
		Environment* env, const DL::Data& data);
};
} // namespace PR
