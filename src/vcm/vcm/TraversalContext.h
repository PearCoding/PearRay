#pragma once

#include "Wavelength.h"
#include "spectral/SpectralBlob.h"

namespace PR {
namespace VCM {
struct BaseTraversalContext {
	SpectralBlob Throughput = SpectralBlob::Ones();
	float MIS_VCM			= 0;
	float MIS_VC			= 0;
	float MIS_VM			= 0;
};

struct CameraTraversalContext : public BaseTraversalContext {
	size_t LightPathID = 0;			 // Selected light path
	SpectralPermutation Permutation; // Permutation used between selected light path and this (only when VM is enabled, as bdpt used the same wavelengths for light and camera path)
	SpectralBlob WavelengthFilter = SpectralBlob::Ones();
};

struct LightTraversalContext : public BaseTraversalContext {
	size_t LightPathID	 = 0; // ID of light path
	size_t LightPathSize = 0;
	bool IsFiniteLight	 = false;
};
} // namespace VCM
} // namespace PR