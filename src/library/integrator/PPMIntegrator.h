#pragma once

#include "Integrator.h"
#include "photon/Photon.h"
#include "photon/PhotonMap.h"
#include "renderer/OutputChannel.h"
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

	void init() override;
	Spectrum apply(const Ray& in, RenderTile* tile, uint32 pass, ShaderClosure& sc) override;

	void onStart() override;
	void onNextPass(uint32 pass, bool& clean) override;
	void onEnd() override;
	bool needNextPass(uint32 pass) const override;

	void onPass(RenderTile* tile, uint32 pass) override;

	RenderStatus status() const;

private:
	void photonPass(RenderTile* tile, uint32 pass);
	Spectrum accumPass(const Ray& in, const ShaderClosure& sc, RenderTile* tile);

	Photon::PhotonMap* mPhotonMap;

	struct Light {
		RenderEntity* Entity;
		uint64 Photons;
		float Surface;
		SphereMap* Proj;
	};

	struct LightTileData {
		Light* Entity;
		uint64 Photons;
	};

	struct TileData {
		std::vector<LightTileData> Lights;
		uint64 PhotonsEmitted;
		uint64 PhotonsStored;
	} * mTileData;

	std::shared_ptr<OutputSpectral> mAccumulatedFlux;
	std::shared_ptr<Output1D> mSearchRadius2;
	std::shared_ptr<OutputCounter> mLocalPhotonCount;

	std::vector<Light> mLights;
	uint32 mProjMaxTheta;
	uint32 mProjMaxPhi;

	uint64 mMaxPhotonsStoredPerPass;
};
}
