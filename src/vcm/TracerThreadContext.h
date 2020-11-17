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
	inline TracerThreadContext(const BoundingBox& bbox, float scene_grid_delta, const Options& options)
		: TracerThreadContext(options)
	{
		LightMap = std::make_unique<PathVertexMap>(bbox, scene_grid_delta, LightVertices);
	}

	inline TracerThreadContext(const Options& options)
		: LightVertices()
		, LightPathEnds()
		, LightMap()
		, CameraPath(options.MaxCameraRayDepthHard + 2)
		, LightPath(options.MaxLightRayDepthHard + 2)
		, TmpPath(options.MaxCameraRayDepthHard + options.MaxLightRayDepthHard + 2)
	{
		CameraPath.addToken(LightPathToken::Camera());
	}

	std::vector<PathVertex> LightVertices; // Only used for non merging events
	std::vector<size_t> LightPathEnds;	   // Setups ends of the light path
	inline std::pair<std::vector<PathVertex>::const_iterator, std::vector<PathVertex>::const_iterator>
	lightPath(size_t path) const
	{
		if (path >= LightPathEnds.size())
			return { LightVertices.end(), LightVertices.end() };

		if (path == 0)
			return { LightVertices.begin(), LightVertices.begin() + LightPathEnds[path] };
		else
			return { LightVertices.begin() + LightPathEnds[path - 1], LightVertices.begin() + LightPathEnds[path] };
	}

	inline const PathVertex* lightVertex(size_t path, size_t index) const
	{
		if (path >= LightPathEnds.size())
			return nullptr;

		if (path == 0)
			return &LightVertices[index];
		else
			return &LightVertices[LightPathEnds[path - 1] + index];
	}

	std::unique_ptr<PathVertexMap> LightMap;

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
		LightVertices.clear();
		LightPathEnds.clear();
	}
};
} // namespace VCM
} // namespace PR