#pragma once

#include "shader/ShadingContext.h"

namespace PR {

enum class NodeType {
	FloatScalar	  = 0,
	FloatSpectral = 1,
	FloatVector	  = 2
};

class INode {
public:
	virtual ~INode() = default;

	virtual std::string dumpInformation() const = 0;
	inline NodeType type() const { return mType; }

protected:
	inline explicit INode(NodeType type)
		: mType(type)
	{
	}

private:
	const NodeType mType;
};

///////////////////
class FloatScalarNode : public INode {
public:
	inline FloatScalarNode()
		: INode(NodeType::FloatScalar)
	{
	}
	virtual ~FloatScalarNode() = default;

	virtual float eval(const ShadingContext& ctx) const = 0;
};

///////////////////
class FloatSpectralNode : public INode {
public:
	inline FloatSpectralNode()
		: INode(NodeType::FloatSpectral)
	{
	}
	virtual ~FloatSpectralNode() = default;

	virtual SpectralBlob eval(const ShadingContext& ctx) const = 0;
	virtual Vector2i queryRecommendedSize() const			   = 0;
};

///////////////////
class FloatVectorNode : public INode {
public:
	inline FloatVectorNode()
		: INode(NodeType::FloatVector)
	{
	}
	virtual ~FloatVectorNode() = default;

	virtual Vector3f eval(const ShadingContext& ctx) const = 0;
};
} // namespace PR
