#pragma once

#include "entity/VirtualEntity.h"
#include "emission/IEmission.h"
#include "math/SIMD.h"
#include <memory>

namespace PR {
class RenderTileSession;

class PR_LIB IInfiniteLight : public VirtualEntity {
public:
	IInfiniteLight(uint32 id, const std::string& name);
	virtual ~IInfiniteLight() {}

	virtual void startGroup(size_t size, const RenderTileSession& session) = 0;
	virtual void endGroup()											   = 0;

	/*
		Evaluate the light based on incident direction and point information.
	*/
	virtual void eval(const LightEvalInput& in, LightEvalOutput& out, const RenderTileSession& session) const = 0;

	virtual std::string dumpInformation() const;
};
} // namespace PR

