#pragma once

#include "material/MaterialType.h"
#include "shader/ShadingContext.h"

namespace PR {

enum class NodeFlag {
	Const			= 0x1,
	TextureVarying	= 0x2,
	SpectralVarying = 0x4,	// Wavelength (explictly) used
	SpatialVarying	= 0x8,	// Position used
	TimeVarying		= 0x10, // Time used
};
PR_MAKE_FLAGS(NodeFlag, NodeFlags)

enum class NodeType {
	FloatScalar	  = 0,
	FloatSpectral = 1,
	FloatVector	  = 2
};

using WavelengthRange = std::pair<float, float>;
/// Merge two wavelength ranges and respect the given unbounded information. 
/// An unbounded value will be replaced if not both values of the same side are unbounded
inline WavelengthRange unionRange(const WavelengthRange& a, const WavelengthRange& b)
{
	return {
		a.first < 0 ? b.first : (b.first < 0 ? a.first : std::min(a.first, b.first)),
		std::max(a.second, b.second)
	};
}

class INode {
public:
	virtual ~INode() = default;

	virtual std::string dumpInformation() const = 0;
	inline NodeType type() const { return mType; }
	inline NodeFlags flags() const { return mFlags; }
	inline MaterialSampleFlags materialFlags() const { return materialFlags(flags()); }

	inline static MaterialSampleFlags materialFlags(NodeFlags flags)
	{
		MaterialSampleFlags m = 0;
		if (flags & NodeFlag::SpectralVarying)
			m |= MaterialSampleFlag::SpectralVarying;
		if (flags & NodeFlag::SpatialVarying)
			m |= MaterialSampleFlag::SpatialVarying;
		if (flags & NodeFlag::TimeVarying)
			m |= MaterialSampleFlag::TimeVarying;
		return m;
	}

	/// Return the wavelength boundary this node prefers to operates in.
	/// A negative values means unbounded, and the global wavelength range shall be used if necessary
	/// Keep in mind that a node has to operate on all wavelengths and should return 0 if called for wavelengths outside the given range
	inline virtual WavelengthRange wavelengthRange() const { return { -1.0f, -1.0f }; }

protected:
	inline explicit INode(NodeFlags flags, NodeType type)
		: mType(type)
		, mFlags(flags)
	{
	}

private:
	const NodeType mType;
	const NodeFlags mFlags;
};

///////////////////
class FloatScalarNode : public INode {
public:
	inline FloatScalarNode(NodeFlags flags)
		: INode(flags, NodeType::FloatScalar)
	{
	}
	virtual ~FloatScalarNode() = default;

	virtual float eval(const ShadingContext& ctx) const = 0;
};

///////////////////
class FloatSpectralNode : public INode {
public:
	inline FloatSpectralNode(NodeFlags flags)
		: INode(flags, NodeType::FloatSpectral)
	{
	}
	virtual ~FloatSpectralNode() = default;

	virtual SpectralBlob eval(const ShadingContext& ctx) const = 0;
	virtual Vector2i queryRecommendedSize() const			   = 0;
};

///////////////////
class FloatVectorNode : public INode {
public:
	inline FloatVectorNode(NodeFlags flags)
		: INode(flags, NodeType::FloatVector)
	{
	}
	virtual ~FloatVectorNode() = default;

	virtual Vector3f eval(const ShadingContext& ctx) const = 0;
};
} // namespace PR
