#pragma once

#include "shader/ShaderOutput.h"

namespace PR {
class PR_LIB ConstScalarShaderOutput : public PR::ScalarShaderOutput {
public:
	explicit ConstScalarShaderOutput(float f);
	void eval(float& f, const PR::ShaderClosure& point) const override;

private:
	float mValue;
};
} // namespace PR
