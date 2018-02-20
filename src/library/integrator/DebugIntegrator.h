#pragma once

#include "OnePassIntegrator.h"

namespace PR {
class PR_LIB DebugIntegrator : public OnePassIntegrator {
public:
	explicit DebugIntegrator(RenderContext* renderer);

	void init() override;

protected:
	void onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session) override;

private:
	const DebugMode mDebugMode;
};
}
