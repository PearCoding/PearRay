#pragma once

#include "IObject.h"
#include "shader/ShadingContext.h"

namespace PR {

enum NodeType {
	NT_FloatScalar	 = 0,
	NT_FloatSpectral = 1,
	NT_FloatVector	 = 2
};

class INode : public IObject {
public:
	virtual ~INode() = default;

	virtual std::string dumpInformation() const = 0;
	inline NodeType type() const { return mType; }

protected:
	inline explicit INode(NodeType type)
		: IObject()
		, mType(type)
	{
	}

private:
	const NodeType mType;
};

///////////////////
class FloatScalarNode : public INode {
public:
	inline FloatScalarNode()
		: INode(NT_FloatScalar)
	{
	}
	virtual ~FloatScalarNode() = default;

	virtual float eval(const ShadingContext& ctx) const = 0;
};

///////////////////
class FloatSpectralNode : public INode {
public:
	inline FloatSpectralNode()
		: INode(NT_FloatSpectral)
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
		: INode(NT_FloatVector)
	{
	}
	virtual ~FloatVectorNode() = default;

	virtual Vector3f eval(const ShadingContext& ctx) const = 0;
};
} // namespace PR
