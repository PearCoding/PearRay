#pragma once

#include "shader/Socket.h"

namespace DL {
class Data;
} // namespace DL

namespace PR {
class Environment;
class SocketParser {
public:
	static std::shared_ptr<FloatSpectralShadingSocket> getSpectralOutput(
		Environment* env, const DL::Data& data, bool allowScalar = false);
	static std::shared_ptr<FloatSpectralMapSocket> getSpectralMapOutput(
		Environment* env, const DL::Data& data, bool allowScalar = false);
	static std::shared_ptr<FloatScalarShadingSocket> getScalarOutput(
		Environment* env, const DL::Data& data);
	static std::shared_ptr<FloatVectorShadingSocket> getVectorOutput(
		Environment* env, const DL::Data& data);
};
} // namespace PR
