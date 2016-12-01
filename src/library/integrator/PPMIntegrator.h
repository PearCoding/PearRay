#pragma once

#include "Integrator.h"
#include "photon/Photon.h"
#include "photon/PointMap.h"
#include "shader/ShaderClosure.h"

#include <deque>
#include <list>

namespace PR
{
	class Material;
	class RenderEntity;
	class SphereMap;

	/*
	 * Progressive Photon Mapping: A Probabilistic Approach
	 * by Claude Knaus and Matthias Zwicker
	 */
	class PR_LIB PPMIntegrator : public Integrator
	{
	public:
		PPMIntegrator();
		virtual ~PPMIntegrator();

		void init(RenderContext* renderer) override;
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

		virtual uint64 maxSamples(const RenderContext* renderer) const override;
		virtual uint64 maxPasses(const RenderContext* renderer) const override;

	private:
		Spectrum firstPass(const Spectrum& weight, const Ray& in, const ShaderClosure& sc, RenderThreadContext* context);

		struct RayHitPoint
		{
			ShaderClosure SC;

			uint32 PixelX, PixelY;
#if PR_PHOTON_RGB_MODE == 2
			float Weight[3];
#else
			Spectrum Weight;
#endif

			// Will be updated!
			float CurrentRadius;
			uint64 CurrentPhotons;
#if PR_PHOTON_RGB_MODE == 2
			float CurrentFlux[3];
#else
			Spectrum CurrentFlux;
#endif
		};

		RenderContext* mRenderer;
		Photon::PointMap<Photon::Photon>* mPhotonMap;

		struct ThreadData
		{
			std::deque<RayHitPoint> HitPoints;
			Photon::PointSphere<Photon::Photon> PhotonSearchSphere;
		}* mThreadData;

		struct Light
		{
			RenderEntity* Entity;
			uint64 Photons;
			float Surface;
			SphereMap* Proj;
		};
		std::list<Light*> mLights;
		uint32 mProjMaxTheta;
		uint32 mProjMaxPhi;
	};
}