#pragma once

#include "OnePassIntegrator.h"
#include <vector>

namespace PR {
struct ShaderClosure;
class PR_LIB DirectIntegrator : public OnePassIntegrator {
public:
	explicit DirectIntegrator(RenderContext* renderer);

	void init() override;

protected:
	void onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session) override;

private:
	void applyRay(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffbounces,
		ShaderClosure& sc);

	std::vector<struct DI_ThreadData> mThreadData;
};
}
