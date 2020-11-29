#pragma once

#include "shader/ShadingContext.h"
#include <memory>

namespace PR {
class RenderTileSession;

// Evaluation
struct PR_LIB_CORE EmissionEvalInput {
	PR::ShadingContext ShadingContext;
	const class IEntity* Entity;
};

struct PR_LIB_CORE EmissionEvalOutput {
	SpectralBlob Radiance;
};

class PR_LIB_CORE IEmission {
public:
	/// Evaluate the light based on incident direction and point information.
	virtual void eval(const EmissionEvalInput& in, EmissionEvalOutput& out, const RenderTileSession& session) const = 0;

	/// Return average power (W/m^2) for given wavelengths
	virtual SpectralBlob power(const SpectralBlob& wvl) const = 0;

	/// Dump information
	virtual std::string dumpInformation() const;
};
} // namespace PR

