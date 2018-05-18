#pragma once

#include "shader/ShaderOutput.h"

namespace PR {
class PR_LIB ConstVectorShaderOutput : public PR::VectorShaderOutput {
public:
	explicit ConstVectorShaderOutput(const Eigen::Vector3f& spec);
	void eval(Eigen::Vector3f& p, const PR::ShaderClosure& point) const override;

private:
	Eigen::Vector3f mValue;
};
} // namespace PR
