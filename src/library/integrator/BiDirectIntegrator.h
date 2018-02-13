#pragma once

#include "OnePassIntegrator.h"
#include "shader/ShaderClosure.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB BiDirectIntegrator : public OnePassIntegrator {
public:
	explicit BiDirectIntegrator(RenderContext* renderer);
	~BiDirectIntegrator();

	void init() override;
	void apply(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 pass, ShaderClosure& sc) override;

private:
	void applyRay(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffBounces, ShaderClosure& sc);

	std::vector<struct BIDI_TileData> mTileData;
	std::vector<struct BIDI_ThreadData> mThreadData;
};
}
