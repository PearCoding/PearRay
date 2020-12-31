#pragma once

#include "trace/IntersectionPoint.h"

namespace PR {
namespace VCM {
class PathVertex {
public:
	IntersectionPoint IP;	 // IP used to construct this vertex
	SpectralBlob Throughput; // From source to this vertex

	float MIS_VCM; // MIS quantity used for vertex connection and merging
	float MIS_VC;  // MIS quantity used for vertex connection
	float MIS_VM;  // MIS quantity used for vertex merging

	inline uint32 pathLength() const { return IP.Ray.IterationDepth + 1; }
	inline const Vector3f& position() const { return IP.P; }
	inline bool fromLight() const { return IP.Ray.Flags & RayFlag::Light; }
};
} // namespace VCM
} // namespace PR