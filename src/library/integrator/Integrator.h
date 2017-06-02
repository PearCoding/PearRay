#pragma once

#include "renderer/RenderStatus.h"

namespace PR {
class Ray;
class RenderContext;
class RenderEntity;
class RenderTile;
class Spectrum;
struct ShaderClosure;
class PR_LIB Integrator {
public:
	explicit Integrator(RenderContext* renderer)
		: mRenderer(renderer)
	{
	}

	virtual ~Integrator() {}

	virtual void init() = 0;

	virtual void onStart() = 0;
	virtual void onNextPass(uint32 i, bool& clean) = 0; // Not the main thread!
	virtual void onEnd()					  = 0;
	virtual bool needNextPass(uint32 i) const = 0;

	// Per thread
	virtual void onPass(RenderTile* tile, uint32 pass) = 0;

	virtual Spectrum apply(const Ray& in, RenderTile* tile,
						   uint32 pass, ShaderClosure& sc)
		= 0;

	virtual RenderStatus status() const = 0;

	inline RenderContext* renderer() const { return mRenderer; }

protected:
	Spectrum handleInfiniteLights(const Ray& in, const ShaderClosure& sc, RenderTile* tile, float& full_pdf);
	Spectrum handleSpecularPath(const Ray& in, const ShaderClosure& sc, RenderTile* tile, RenderEntity*& lastEntity);

private:
	RenderContext* mRenderer;
};
}
