#pragma once

#include "shader/ShadingContext.h"

namespace PR {

///////////////////
template <typename T>
class ScalarNode {
public:
	ScalarNode()		   = default;
	virtual ~ScalarNode() = default;

	virtual T eval(const ShadingContext& ctx) const = 0;
	virtual std::string dumpInformation() const	  = 0;
};
using FloatScalarNode = ScalarNode<float>;

///////////////////
template <typename T>
class SpectralNode {
public:
	SpectralNode()		  = default;
	virtual ~SpectralNode() = default;

	virtual SpectralBlobBase<T> eval(const ShadingContext& ctx) const = 0;
	virtual Vector2i queryRecommendedSize() const		 = 0;
	virtual std::string dumpInformation() const			 = 0;
};

using FloatSpectralNode = SpectralNode<float>;

///////////////////
template <typename T>
class VectorNode {
public:
	VectorNode()		   = default;
	virtual ~VectorNode() = default;

	virtual Vector3f eval(const ShadingContext& ctx) const = 0;
	virtual std::string dumpInformation() const				= 0;
};

using FloatVectorNode = VectorNode<float>;
} // namespace PR
