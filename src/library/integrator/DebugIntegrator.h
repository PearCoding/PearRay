#pragma once

#include "OnePassIntegrator.h"

namespace PR {
class PR_LIB DebugIntegrator : public OnePassIntegrator {
public:
	explicit DebugIntegrator(RenderContext* renderer);

	void init() override;
	Spectrum apply(const Ray& in, RenderTile* tile, uint32 pass, ShaderClosure& sc) override;
};
}
