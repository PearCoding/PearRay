#pragma once

#include "entity/VirtualEntity.h"
#include "shader/ShadingPoint.h"

namespace PR {
struct PR_LIB_INLINE InfiniteLightEvalInput {
	ShadingPoint Point;
};

struct PR_LIB_INLINE InfiniteLightEvalOutput {
	ColorTriplet Weight;
	float PDF_S;
};

struct PR_LIB_INLINE InfiniteLightSampleInput {
	ShadingPoint Point;
	Vector2f RND;
};

struct PR_LIB_INLINE InfiniteLightSampleOutput {
	ColorTriplet Weight;
	float PDF_S;
	Vector3f Outgoing;
};

class RenderTileSession;
class PR_LIB IInfiniteLight : public VirtualEntity {
public:
	IInfiniteLight(uint32 id, const std::string& name);
	virtual ~IInfiniteLight() {}

	virtual bool hasDeltaDistribution() const { return false; }

	virtual void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out, const RenderTileSession& session) const = 0;
	/*
		Sample the light with point information.
	*/
	virtual void sample(const InfiniteLightSampleInput& in, InfiniteLightSampleOutput& out, const RenderTileSession& session) const = 0;

	virtual std::string dumpInformation() const;
};
} // namespace PR
