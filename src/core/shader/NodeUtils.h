#pragma once

#include "INode.h"

namespace PR {

class PR_LIB_CORE NodeUtils {
public:
	static float average(const SpectralBlob& wvls, const FloatScalarNode* node);
	static SpectralBlob average(const SpectralBlob& wvls, const FloatSpectralNode* node);
};

} // namespace PR
