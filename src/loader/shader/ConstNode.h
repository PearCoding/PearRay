#pragma once

#include "shader/INode.h"
#include "spectral/ParametricBlob.h"

namespace PR {
class PR_LIB_LOADER ConstScalarNode : public FloatScalarNode {
public:
	explicit ConstScalarNode(float f);
	float eval(const ShadingContext& ctx) const override;
	std::string dumpInformation() const override;

private:
	float mValue;
};

class PR_LIB_LOADER ConstSpectralNode : public FloatSpectralNode {
public:
	explicit ConstSpectralNode(const ParametricBlob& f);
	SpectralBlob eval(const ShadingContext& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	ParametricBlob mValue;
};

class PR_LIB_LOADER ConstVectorNode : public FloatVectorNode {
public:
	explicit ConstVectorNode(const Vector3f& f);
	Vector3f eval(const ShadingContext& ctx) const override;
	std::string dumpInformation() const override;

private:
	Vector3f mValue;
};
} // namespace PR
