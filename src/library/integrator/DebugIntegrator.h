#pragma once

#include "OnePassIntegrator.h"

namespace PR {
class PR_LIB DebugIntegrator : public OnePassIntegrator {
public:
	explicit DebugIntegrator(RenderContext* renderer);

	void init() override;
	void apply(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 pass, ShaderClosure& sc) override;
};
}
