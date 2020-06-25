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

ConstSpectralNode::ConstSpectralNode(const ParametricBlob& f)
	: FloatSpectralNode()
	, mValue(f)
{
}

SpectralBlob ConstSpectralNode::eval(const ShadingContext& ctx) const
{
	return SpectralUpsampler::compute(mValue, ctx.WavelengthNM);
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
