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
	const float mValue;
};

class PR_LIB_LOADER ConstSpectralNode : public FloatSpectralNode {
public:
	explicit ConstSpectralNode(const SpectralBlob& f);
	SpectralBlob eval(const ShadingContext& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	const SpectralBlob mValue;
};

class PR_LIB_LOADER ParametricSpectralNode : public FloatSpectralNode {
public:
	explicit ParametricSpectralNode(const ParametricBlob& f);
	SpectralBlob eval(const ShadingContext& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	const ParametricBlob mValue;
};

class PR_LIB_LOADER ParametricScaledSpectralNode : public FloatSpectralNode {
public:
	ParametricScaledSpectralNode(const ParametricBlob& f, float power);
	SpectralBlob eval(const ShadingContext& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	const ParametricBlob mValue;
	const float mPower;
};

class PR_LIB_LOADER SplatSpectralNode : public FloatSpectralNode {
public:
	explicit SplatSpectralNode(const std::shared_ptr<FloatScalarNode>& f);
	SpectralBlob eval(const ShadingContext& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	const std::shared_ptr<FloatScalarNode> mValue;
};

class PR_LIB_LOADER ConstVectorNode : public FloatVectorNode {
public:
	explicit ConstVectorNode(const Vector3f& f);
	Vector3f eval(const ShadingContext& ctx) const override;
	std::string dumpInformation() const override;

private:
	const Vector3f mValue;
};
} // namespace PR
