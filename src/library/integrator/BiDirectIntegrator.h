#pragma once

#include "OnePassIntegrator.h"
#include "shader/ShaderClosure.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB BiDirectIntegrator : public OnePassIntegrator {
public:
	explicit BiDirectIntegrator(RenderContext* renderer);
	~BiDirectIntegrator();

	void init() override;
	Spectrum apply(const Ray& in, RenderThreadContext* context, uint32 pass, ShaderClosure& sc) override;

private:
	Spectrum applyRay(const Ray& in, RenderThreadContext* context, uint32 diffBounces, ShaderClosure& sc);

	struct ThreadData {
		struct EventVertex {
			Spectrum Flux;
			ShaderClosure SC;
			//float PDF;
		};

		EventVertex* LightVertices;
		uint32* LightPathLength;
		//EventVertex* EyeVertices;
	};

	void deleteThreadStructure();

	ThreadData* mThreadData;
	uint32 mThreadCount;
};
}
