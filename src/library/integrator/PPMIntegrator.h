#pragma once

#include "Integrator.h"
#include "photon/Photon.h"
#include "photon/PhotonMap.h"
#include "shader/ShaderClosure.h"
#include "renderer/OutputChannel.h"

#include <deque>
#include <vector>

namespace PR
{
	class Material;
	class RenderEntity;
	class SphereMap;

	/*
	 * Stochastic Progressive Photon Mapping
	 * (Toshiya Hachisuka and Henrik Wann Jensen)
	 */
	class PR_LIB PPMIntegrator : public Integrator
	{
	public:
		PPMIntegrator(RenderContext* renderer);
		virtual ~PPMIntegrator();

		void init() override;
		Spectrum apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc) override;

		void onStart() override;
		void onNextPass(uint32 pass, bool& clean) override;
		void onEnd() override;
		bool needNextPass(uint32 pass) const override;

		void onThreadStart(RenderThreadContext* context) override;
		void onPrePass(RenderThreadContext* context, uint32 i) override;
		void onPass(RenderTile* tile, RenderThreadContext* context, uint32 pass) override;
		void onPostPass(RenderThreadContext* context, uint32 i) override;
		void onThreadEnd(RenderThreadContext* context) override;

		RenderStatus status() const;
	private:
		void photonPass(RenderThreadContext* context, uint32 pass);
		Spectrum accumPass(const Ray& in, const ShaderClosure& sc, RenderThreadContext* context);

		Photon::PhotonMap* mPhotonMap;

		struct Light
		{
			RenderEntity* Entity;
			uint64 Photons;
			float Surface;
			SphereMap* Proj;
		};

		struct LightThreadData
		{
			Light* Entity;
			uint64 Photons;
		};

		struct ThreadData
		{
			std::vector<LightThreadData> Lights;
		}* mThreadData;

		std::shared_ptr<OutputSpectral> mAccumulatedFlux;
		std::shared_ptr<Output1D> mSearchRadius2;
		std::shared_ptr<OutputCounter> mLocalPhotonCount;

		std::vector<Light> mLights;
		uint32 mProjMaxTheta;
		uint32 mProjMaxPhi;

		uint64 mMaxPhotonsStoredPerPass;
		uint64 mPhotonsEmitted;
		uint64 mPhotonsStored;
	};
}
