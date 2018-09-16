#pragma once

#include "OnePassIntegrator.h"

namespace PR {
struct ShaderClosure;
class PR_LIB AOIntegrator : public OnePassIntegrator {
public:
	explicit AOIntegrator(RenderContext* renderer);
	virtual ~AOIntegrator();

	void init() override;

protected:
	void onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session) override;

private:
	uint32 mMaxAOSamples;
	bool mUseMaterials;
};
}
