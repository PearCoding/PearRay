#pragma once

#include "spectral/SpectralBlob.h"

namespace PR {
namespace VCM {
struct CameraTraversalContext {
	SpectralBlob Throughput;
	float MIS_VCM;
	float MIS_VC;
	float MIS_VM;
};

struct LightTraversalContext : public CameraTraversalContext {
	bool IsFiniteLight;
};
} // namespace VCM
} // namespace PR