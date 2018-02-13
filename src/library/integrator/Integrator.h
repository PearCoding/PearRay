#pragma once

#include "renderer/RenderStatus.h"
#include <vector>


namespace PR {
class Ray;
class RenderContext;
class RenderEntity;
class RenderSession;
class Spectrum;
struct ShaderClosure;
class PR_LIB Integrator {
public:
	explicit Integrator(RenderContext* renderer);
	virtual ~Integrator();

	virtual void init();

	virtual void onStart() = 0;
	virtual void onNextPass(uint32 i, bool& clean) = 0; // Not the main thread!
	virtual void onEnd()					  = 0;
	virtual bool needNextPass(uint32 i) const = 0;

	// Per thread
	virtual void onPass(const RenderSession& session, uint32 pass) = 0;

	virtual void apply(Spectrum& spec, const Ray& in, const RenderSession& session,
						   uint32 pass, ShaderClosure& sc)
		= 0;

	virtual RenderStatus status() const = 0;

	inline RenderContext* renderer() const { return mRenderer; }

protected:
	void handleInfiniteLights(Spectrum& spec, const Ray& in, const ShaderClosure& sc, const RenderSession& session, float& full_pdf);
	void handleSpecularPath(Spectrum& spec, const Ray& in, const ShaderClosure& sc, const RenderSession& session, RenderEntity*& lastEntity);

private:
	RenderContext* mRenderer;
	std::vector<struct I_ThreadData> mThreadData;
};
}
