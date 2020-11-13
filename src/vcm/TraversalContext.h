#pragma once

#include "spectral/SpectralBlob.h"

namespace PR {
namespace VCM {
struct BaseTraversalContext {
	SpectralBlob Throughput;
	float MIS_VCM;
	float MIS_VC;
	float MIS_VM;
};

struct CameraTraversalContext : public BaseTraversalContext {
};

struct LightTraversalContext : public BaseTraversalContext {
	bool IsFiniteLight;
};
} // namespace VCM
} // namespace PR