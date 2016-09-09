#pragma once

#include "Integrator.h"

#include <list>

namespace PR
{
	namespace Photon
	{
		class PhotonMap;
		struct PhotonSphere;
	}

	/*
	 * Progressive Photon Mapping: A Probabilistic Approach
	 * by Claude Knaus and Matthias Zwicker
	 */
	class PR_LIB PPMIntegrator : public Integrator
	{
	public:
		PPMIntegrator();
		virtual ~PPMIntegrator();

		void init(Renderer* renderer) override;
		Spectrum apply(const Ray& in, RenderContext* context, uint32 pass) override;
		
		void onStart() override;
		void onNextPass(uint32 i) override;
		void onEnd() override;
		bool needNextPass(uint32 i) const override;

		void onThreadStart(RenderContext* context) override;
		void onPrePass(RenderContext* context, uint32 i) override;
		void onPass(RenderTile* tile, RenderContext* context, uint32 i) override;
		void onPostPass(RenderContext* context, uint32 i) override;
		void onThreadEnd(RenderContext* context) override;

		virtual uint64 maxSamples(const Renderer* renderer) const override;

	private:
		Spectrum applyRay(const Ray& in, const SamplePoint& point, RenderContext* context, uint32 pass);

		Renderer* mRenderer;

		Photon::PhotonMap* mPhotonMap;
		Photon::PhotonSphere* mPhotonSpheres;

		float mCurrentPassRadius2;

		struct Light
		{
			RenderEntity* Entity;
			uint64 Photons;
		};
		std::list<Light*> mLights;
	};
}