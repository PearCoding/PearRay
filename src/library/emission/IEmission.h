#pragma once

#include "IFreezable.h"
#include "math/SIMD.h"
#include "shader/ShadingPoint.h"
#include <memory>

namespace PR {
class RenderTileSession;

// Evaluation
struct PR_LIB_INLINE LightEvalInput {
	ShadingPoint Point;
	class IEntity* Entity;
};

struct PR_LIB_INLINE LightEvalOutput {
	float Weight;
};

class PR_LIB IEmission : public IFreezable {
public:
	IEmission(uint32 id);
	virtual ~IEmission() {}

	inline uint32 id() const;

	/*virtual void startGroup(size_t size, const RenderTileSession& session) = 0;
	virtual void endGroup()											   = 0;*/

	/*
		Evaluate the light based on incident direction and point information.
	*/
	virtual void eval(const LightEvalInput& in, LightEvalOutput& out, const RenderTileSession& session) const = 0;

	virtual std::string dumpInformation() const;

private:
	uint32 mID;
};
} // namespace PR

#include "IEmission.inl"
