#pragma once

#include "OnePassIntegrator.h"
#include "material/Material.h"
#include <vector>

namespace PR {
struct ShaderClosure;
class PR_LIB DirectIntegrator : public OnePassIntegrator {
public:
	explicit DirectIntegrator(RenderContext* renderer);
	virtual ~DirectIntegrator();

	void init() override;

protected:
	void onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session) override;

private:
	void applyRay(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffbounces,
		ShaderClosure& sc);

	void addDirectContribution(Spectrum& spec, const Ray& in, const RenderSession& session, const ShaderClosure& sc);

	bool addIndirectContribution(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffbounces, const ShaderClosure& sc);

	bool sampleHemisphere(Spectrum& spec, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, uint32 diffbounces, const ShaderClosure& sc);

	void sampleLight(Spectrum& spec, RenderEntity* light, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, const ShaderClosure& sc);

	uint32 mMaxLightSamples;
	uint32 mLightCount;
	uint32 mMaxDiffBounces;
	std::vector<struct DI_ThreadData> mThreadData;
};
}
