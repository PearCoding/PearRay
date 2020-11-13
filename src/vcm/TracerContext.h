#pragma once

#include "Options.h"
#include "PathVertex.h"
#include "path/LightPath.h"

namespace PR {
class RenderTileSession;

namespace VCM {

// This is only useful for bdpt contexts
// Data for one thread
class TracerContext {
public:
	inline explicit TracerContext(RenderTileSession& session, const Options& options)
		: CameraPath(options.MaxCameraRayDepthHard + 2)
		, LightPath(options.MaxLightRayDepthHard + 2)
		, TmpPath(options.MaxCameraRayDepthHard + options.MaxLightRayDepthHard + 2)
		, Session(session)
	{
		CameraPath.addToken(LightPathToken::Camera());
		LightVertices.reserve(options.MaxLightRayDepthHard + 1); // TODO: This only works for bidi
	}

	std::vector<PathVertex> LightVertices;

	// Context of evaluation
	PR::LightPath CameraPath;
	PR::LightPath LightPath;
	PR::LightPath TmpPath;

	RenderTileSession& Session;
};
} // namespace VCM
} // namespace PR