#pragma once

#include "spectral/SpectralBlob.h"

namespace PR {
namespace Photon {
// 14 Floats :O
struct alignas(16) Photon {
	Vector3f Position;
	Vector3f Direction;
	SpectralBlobStorage Power;
	SpectralBlobStorage WavelengthNM;
};
} // namespace Photon
} // namespace PR
