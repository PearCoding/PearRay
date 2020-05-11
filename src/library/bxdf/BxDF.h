#pragma once

#include "BxDFData.h"
#include "BxDFType.h"

namespace PR {
class PR_LIB BxDF {
	PR_CLASS_STACK_ONLY(BxDF);

public:
	virtual void eval(const BxDFEval& s) const	   = 0;
	virtual void sample(const BxDFSample& s) const = 0;

	virtual BxDFType type() const = 0;
};
} // namespace PR