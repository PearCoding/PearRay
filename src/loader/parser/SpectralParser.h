#pragma once

#include "PR_Config.h"

namespace DL {
class Data;
} // namespace DL

namespace PR {
class SpectralUpsampler;
class FloatSpectralNode;
class SpectralParser {
public:
	static std::shared_ptr<FloatSpectralNode> getSpectrum(SpectralUpsampler* upsampler, const DL::Data& data);
};
} // namespace PR
