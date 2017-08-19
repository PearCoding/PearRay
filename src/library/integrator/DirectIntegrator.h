#pragma once

#include "OnePassIntegrator.h"

namespace PR {
struct ShaderClosure;
class PR_LIB DirectIntegrator : public OnePassIntegrator {
public:
	explicit DirectIntegrator(RenderContext* renderer);

	void init() override;
	Spectrum apply(const Ray& in, RenderTile* tile, uint32 pass,
				   ShaderClosure& sc) override;
};
}
