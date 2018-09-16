#pragma once

#include "IFreezable.h"
#include "math/SIMD.h"
#include <memory>

namespace PR {
class RenderSession;

// Evaluation
struct PR_LIB_INLINE LightEvalInput {
	uint32 EntityID; // Always just one entity

	vfloat Incident[3];
	vfloat UV[2];
	vuint32 WavelengthIndex;
	vuint32 PrimitiveID;
};

struct PR_LIB_INLINE LightEvalOutput {
	vfloat Weight;
};

class PR_LIB ILight : public IFreezable {
public:
	ILight(uint32 id);
	virtual ~ILight() {}

	inline uint32 id() const;

	virtual void startGroup(size_t size, const RenderSession& session) = 0;
	virtual void endGroup()											   = 0;

	/*
		Evaluate the light based on incident direction and point information.
	*/
	virtual void eval(const LightEvalInput& in, LightEvalOutput& out, const RenderSession& session) const = 0;

	virtual std::string dumpInformation() const;

private:
	uint32 mID;
};
} // namespace PR

#include "ILight.inl"
