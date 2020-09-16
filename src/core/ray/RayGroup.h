#pragma once

#include "spectral/SpectralBlob.h"

namespace PR {

class PR_LIB_CORE RayGroup {
public:
	SpectralBlob Importance	   = SpectralBlob::Zero(); // Initial importance (camera importanceu)
	SpectralBlob WavelengthNM  = SpectralBlob::Zero(); // Initial wavelength (which is the same in our case all the time)
	SpectralBlob WavelengthPDF = SpectralBlob::Ones();
	float Time				   = 0;
	float TimePDF			   = 1;
};

} // namespace PR