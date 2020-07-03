#include "ConstNode.h"
#include "spectral/SpectralUpsampler.h"

#include <sstream>

namespace PR {
ConstScalarNode::ConstScalarNode(float f)
	: FloatScalarNode()
	, mValue(f)
{
}

float ConstScalarNode::eval(const ShadingContext&) const
{
	return mValue;
}

std::string ConstScalarNode::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue;
	return sstream.str();
}

/////////////////////////////////////

ConstSpectralNode::ConstSpectralNode(const SpectralBlob& f)
	: FloatSpectralNode()
	, mValue(f)
{
}

SpectralBlob ConstSpectralNode::eval(const ShadingContext&) const
{
	return mValue;
}

Vector2i ConstSpectralNode::queryRecommendedSize() const
{
	return Vector2i(1, 1);
}

std::string ConstSpectralNode::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue;
	return sstream.str();
}

/////////////////////////////////////

ParametricSpectralNode::ParametricSpectralNode(const ParametricBlob& f)
	: FloatSpectralNode()
	, mValue(f)
{
}

SpectralBlob ParametricSpectralNode::eval(const ShadingContext& ctx) const
{
	return SpectralUpsampler::compute(mValue, ctx.WavelengthNM);
}

Vector2i ParametricSpectralNode::queryRecommendedSize() const
{
	return Vector2i(1, 1);
}

std::string ParametricSpectralNode::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue;
	return sstream.str();
}

/////////////////////////////////////

ParametricScaledSpectralNode::ParametricScaledSpectralNode(const ParametricBlob& f, float power)
	: FloatSpectralNode()
	, mValue(f)
	, mPower(power)
{
}

SpectralBlob ParametricScaledSpectralNode::eval(const ShadingContext& ctx) const
{
	return SpectralUpsampler::compute(mValue, ctx.WavelengthNM) * mPower;
}

Vector2i ParametricScaledSpectralNode::queryRecommendedSize() const
{
	return Vector2i(1, 1);
}

std::string ParametricScaledSpectralNode::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue << "x" << mPower;
	return sstream.str();
}

/////////////////////////////////////

SplatSpectralNode::SplatSpectralNode(const std::shared_ptr<FloatScalarNode>& f)
	: FloatSpectralNode()
	, mValue(f)
{
}

SpectralBlob SplatSpectralNode::eval(const ShadingContext& ctx) const
{
	return SpectralBlob(mValue->eval(ctx));
}

Vector2i SplatSpectralNode::queryRecommendedSize() const
{
	return Vector2i(1, 1);
}

std::string SplatSpectralNode::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue->dumpInformation();
	return sstream.str();
}

/////////////////////////////////////

ConstVectorNode::ConstVectorNode(const Vector3f& f)
	: FloatVectorNode()
	, mValue(f)
{
}

Vector3f ConstVectorNode::eval(const ShadingContext&) const
{
	return mValue;
}

std::string ConstVectorNode::dumpInformation() const
{
	std::stringstream sstream;
	sstream << mValue;
	return sstream.str();
}
} // namespace PR