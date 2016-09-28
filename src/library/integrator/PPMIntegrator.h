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
		void onNextPass(uint32 pass, bool& clean) override;
		void onEnd() override;
		bool needNextPass(uint32 pass) const override;

		void onThreadStart(RenderContext* context) override;
		void onPrePass(RenderContext* context, uint32 i) override;
		void onPass(RenderTile* tile, RenderContext* context, uint32 pass) override;
		void onPostPass(RenderContext* context, uint32 i) override;
		void onThreadEnd(RenderContext* context) override;

		virtual uint64 maxSamples(const Renderer* renderer) const override;
		virtual uint64 maxPasses(const Renderer* renderer) const override;

	private:
		Spectrum firstPass(const Spectrum& weight, const Ray& in, const ShaderClosure& sc, RenderContext* context);

		struct RayHitPoint
		{
			ShaderClosure SC;

			float PixelX, PixelY;
			Spectrum Weight;

			// Will be updated!
			float CurrentRadius;
			uint64 CurrentPhotons;
			Spectrum CurrentFlux;
		};

		Renderer* mRenderer;
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
		};
		std::list<Light*> mLights;
	};
}