#pragma once

#include "spectral/SpectralBlob.h"

namespace PR {

class PR_LIB_CORE RayGroup {
public:
	float BlendWeight		   = 1;					   // Blending weight to framebuffer
	SpectralBlob Importance	   = SpectralBlob::Zero(); // Initial importance (camera importance)
	SpectralBlob WavelengthNM  = SpectralBlob::Zero(); // Initial wavelength
	SpectralBlob WavelengthPDF = SpectralBlob::Ones();
	float Time				   = 0;
	float TimePDF			   = 1;
};

} // namespace PR