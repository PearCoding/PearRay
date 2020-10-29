#pragma once

#include "INode.h"

namespace PR {

class PR_LIB_CORE NodeUtils {
public:
	static float average(const SpectralBlob& wvls, FloatScalarNode* node);
	static float average(const SpectralBlob& wvls, FloatSpectralNode* node);
};

} // namespace PR
