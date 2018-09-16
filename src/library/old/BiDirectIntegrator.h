#pragma once

#include "OnePassIntegrator.h"
#include "shader/ShaderClosure.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB BiDirectIntegrator : public OnePassIntegrator {
public:
	explicit BiDirectIntegrator(RenderContext* renderer);
	virtual ~BiDirectIntegrator();

	void init() override;

protected:
	void onPixel(Spectrum& spec, ShaderClosure& sc, const Ray& in, const RenderSession& session) override;

private:
	void buildAllLightPaths(const Ray& in, const RenderSession& session);
	void buildLightPath(const Ray& in, RenderEntity* light, uint32 lightNr, const RenderSession& session);
	uint32 buildEyePath(const Ray& in, const RenderSession& session);
	void combineLightEyePath(const Ray& in, Spectrum& spec, uint32 lightNr, uint32 eyeLength, const RenderSession& session);
	float combineLightEyeNode(const Ray& in, Spectrum& spec, uint32 lightNr, uint32 s, uint32 t, const RenderSession& session);

	uint32 mMaxLightSamples;
	uint32 mMaxLightDepth;
	uint32 mMaxCameraDepth;
	uint32 mMaxDiffBounces;
	uint32 mMaxLightPatches;

	std::vector<struct BIDI_ThreadData> mThreadData;
};
}
