#pragma once

#include "shader/ShadingContext.h"
#include "EmissionData.h"

namespace PR {
class RenderTileSession;

class PR_LIB_CORE IEmission {
public:
	/// Evaluate the light based on incident direction and point information.
	virtual void eval(const EmissionEvalInput& in, EmissionEvalOutput& out, const RenderTileSession& session) const = 0;

	/// Compute the pdf of the emission based on outgoing direction and point information.
	/// The calculation is in shading space.
	virtual void pdf(const EmissionEvalInput& in, EmissionPDFOutput& out, const RenderTileSession& session) const = 0;

	/// Sample a direction and, if requested, a wavelength and evaluate.
	/// The calculation and output is in shading space.
	/// In contary to IMaterial::sample, no radiance will be evaluated
	virtual void sample(const EmissionSampleInput& in, EmissionSampleOutput& out, const RenderTileSession& session) const = 0;

	/// Return average power (W/m^2) for given wavelengths
	virtual SpectralBlob power(const SpectralBlob& wvl) const = 0;

	/// Dump information
	virtual std::string dumpInformation() const;
};
} // namespace PR
