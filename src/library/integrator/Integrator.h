#pragma once

#include "PR_Config.h"

namespace PR
{
	class Ray;
	class RenderContext;
	class RenderThreadContext;
	class RenderEntity;
	class RenderTile;
	class Spectrum;
	struct ShaderClosure;
	class PR_LIB Integrator
	{
	public:
		virtual ~Integrator() {}

		virtual void init(RenderContext* renderer) = 0;

		virtual void onStart() = 0;
		virtual void onNextPass(uint32 i, bool& clean) = 0;// Not the main thread!
		virtual void onEnd() = 0;
		virtual bool needNextPass(uint32 i) const = 0;

		// Per thread
		virtual void onThreadStart(RenderThreadContext* context) = 0;
		virtual void onPrePass(RenderThreadContext* context, uint32 pass) = 0;
		virtual void onPass(RenderTile* tile, RenderThreadContext* context, uint32 pass) = 0;
		virtual void onPostPass(RenderThreadContext* context, uint32 pass) = 0;
		virtual void onThreadEnd(RenderThreadContext* context) = 0;

		virtual uint64 maxSamples(const RenderContext* renderer) const = 0;
		virtual uint64 maxPasses(const RenderContext* renderer) const = 0;// Can change over time!

		virtual Spectrum apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc) = 0;

	protected:
		static Spectrum handleInfiniteLights(const Ray& in, const ShaderClosure& sc, RenderThreadContext* context, float& full_pdf);
		static Spectrum handleSpecularPath(const Ray& in, const ShaderClosure& sc, RenderThreadContext* context, RenderEntity*& lastEntity);
	};
}
