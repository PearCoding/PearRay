#pragma once

#include "Integrator.h"
#include "buffer/FrameBuffer.h"
#include "photon/Photon.h"
#include "photon/PhotonMap.h"
#include "shader/ShaderClosure.h"

#include <deque>
#include <vector>

namespace PR {
class Material;
class RenderEntity;
class SphereMap;

/*
* Stochastic Progressive Photon Mapping
* (Toshiya Hachisuka and Henrik Wann Jensen)
*/
class PR_LIB PPMIntegrator : public Integrator {
public:
	explicit PPMIntegrator(RenderContext* renderer);
	virtual ~PPMIntegrator();

	void onStart() override;
	void init() override;
	void apply(Spectrum& spec, const Ray& in, const RenderSession& session, uint32 pass, ShaderClosure& sc) override;

	void onNextPass(uint32 pass, bool& clean) override;
	void onEnd() override;
	bool needNextPass(uint32 pass) const override;

	void onPass(const RenderSession& session, uint32 pass) override;

	RenderStatus status() const;

private:
	void photonPass(const RenderSession& session, uint32 pass);
	Spectrum accumPass(const Ray& in, ShaderClosure& sc, uint32 diffbounces, const RenderSession& session);

	Photon::PhotonMap* mPhotonMap;

	std::shared_ptr<FrameBufferFloat> mAccumulatedFlux;
	std::shared_ptr<FrameBufferFloat> mSearchRadius2;
	std::shared_ptr<FrameBufferUInt64> mLocalPhotonCount;

	std::vector<struct PPM_Light> mLights;
	std::vector<struct PPM_TileData> mTileData;
	std::vector<struct PPM_ThreadData> mThreadData;
	
	uint64 mMaxPhotonsStoredPerPass;
};
}
