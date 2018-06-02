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
	void applyRay(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffBounces, ShaderClosure& sc);

	void addDirectContribution(Spectrum& spec, const Ray& in, const RenderSession& session, const ShaderClosure& sc);

	bool addIndirectContribution(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 diffbounces, const ShaderClosure& sc);

	bool sampleHemisphere(Spectrum& spec, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, uint32 diffbounces, const ShaderClosure& sc);

	void sampleLightPatch(Spectrum& spec, uint32 patch, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, const ShaderClosure& sc);

	void sampleLightPatchFirstNode(Spectrum& spec, float& full_pdf, uint32 index, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, const ShaderClosure& sc);

	void sampleLightPatchOtherNodes(Spectrum& spec, float& full_pdf, uint32 index, const Eigen::Vector3f& rnd, const Ray& in, const RenderSession& session, const ShaderClosure& sc);

	uint32 mMaxLightSamples;
	uint32 mMaxLightDepth;
	uint32 mMaxCameraDepth;
	uint32 mMaxDiffBounces;
	uint32 mMaxLightPatches;

	std::vector<struct BIDI_TileData> mTileData;
	std::vector<struct BIDI_ThreadData> mThreadData;
};
}
