#pragma once

#include "Options.h"
#include "PathVertexMap.h"
#include "container/HashGrid.h"
#include "path/LightPath.h"

namespace PR {
namespace VCM {

// Data for one thread
class TracerThreadContext {
public:
	inline TracerThreadContext(const Options& options)
		: BDPTLightVertices()
		, CameraPath(options.MaxCameraRayDepthHard + 2)
		, LightPath(options.MaxLightRayDepthHard + 2)
		, TmpPath(options.MaxCameraRayDepthHard + options.MaxLightRayDepthHard + 2)
	{
		CameraPath.addToken(LightPathToken::Camera());
	}

	std::vector<PathVertex> BDPTLightVertices; // Only used for non merging bdpt

	// Context of evaluation
	PR::LightPath CameraPath;
	PR::LightPath LightPath;
	PR::LightPath TmpPath;

	inline void resetCamera()
	{
		CameraPath.popTokenUntil(1); // Keep first token
	}

	inline void resetLights()
	{
		LightPath.popTokenUntil(0);
		BDPTLightVertices.clear();
	}
};
} // namespace VCM
} // namespace PR