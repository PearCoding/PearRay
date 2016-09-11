#pragma once

#include "Config.h"

namespace PR
{
	class Ray;
	class Renderer;
	class RenderContext;
	class RenderTile;
	class Spectrum;
	struct ShaderClosure;
	class PR_LIB Integrator
	{
	public:
		virtual ~Integrator() {}

		virtual void init(Renderer* renderer) = 0;

		virtual void onStart() = 0;
		virtual void onNextPass(uint32 i) = 0;// Not the main thread!
		virtual void onEnd() = 0;
		virtual bool needNextPass(uint32 i) const = 0;

		// Per thread
		virtual void onThreadStart(RenderContext* context) = 0;
		virtual void onPrePass(RenderContext* context, uint32 pass) = 0;
		virtual void onPass(RenderTile* tile, RenderContext* context, uint32 pass) = 0;
		virtual void onPostPass(RenderContext* context, uint32 pass) = 0;
		virtual void onThreadEnd(RenderContext* context) = 0;

		virtual uint64 maxSamples(const Renderer* renderer) const = 0;
		virtual Spectrum apply(const Ray& in, RenderContext* context, uint32 pass) = 0;

	protected:
		static Spectrum handleInfiniteLights(const Ray& in, const ShaderClosure& sc, RenderContext* context, float& full_pdf);
	};
}