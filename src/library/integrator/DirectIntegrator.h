#pragma once

#include "OnePassIntegrator.h"
#include <vector>

namespace PR {
struct ShaderClosure;
class PR_LIB DirectIntegrator : public OnePassIntegrator {
public:
	explicit DirectIntegrator(RenderContext* renderer);

	void init() override;
	void apply(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 pass,
				   ShaderClosure& sc) override;

private:
	void applyRay(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffbounces,
		ShaderClosure& sc);

	std::vector<struct DI_ThreadData> mThreadData;
};
}
