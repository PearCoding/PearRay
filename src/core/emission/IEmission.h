#pragma once

#include "IObject.h"
#include "shader/ShadingContext.h"
#include <memory>

namespace PR {
class RenderTileSession;

// Evaluation
struct PR_LIB_CORE EmissionEvalInput {
	PR::ShadingContext ShadingContext;
	class IEntity* Entity;
};

struct PR_LIB_CORE EmissionEvalOutput {
	SpectralBlob Weight;
};

class PR_LIB_CORE IEmission : public IObject {
public:
	IEmission(uint32 id);
	virtual ~IEmission() {}

	inline uint32 id() const;

	/*virtual void startGroup(size_t size, const RenderTileSession& session) = 0;
	virtual void endGroup()											   = 0;*/

	/*
		Evaluate the light based on incident direction and point information.
	*/
	virtual void eval(const EmissionEvalInput& in, EmissionEvalOutput& out, const RenderTileSession& session) const = 0;

	virtual std::string dumpInformation() const;

private:
	uint32 mID;
};
} // namespace PR

#include "IEmission.inl"
