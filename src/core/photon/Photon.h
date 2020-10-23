#pragma once

#include "spectral/SpectralBlob.h"

namespace PR {
namespace Photon {
// 14 Floats :O
struct alignas(16) Photon {
	float Position[3];
	float Direction[3];
	SpectralBlob Power;
	SpectralBlob Wavelengths;
};
} // namespace Photon
} // namespace PR
