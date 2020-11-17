#pragma once

#include "container/HashGrid.h"
#include "PathVertex.h"

namespace PR {
namespace VCM {
template <typename T>
struct vcm_position_getter;

template <>
struct vcm_position_getter<size_t> {
	const std::vector<PathVertex>& Vertices;
	inline Vector3f operator()(size_t id) const
	{
		return Vertices[id].IP.P;
	}
};

struct QuerySphere // Setup for the estimation query
{
	Vector3f Normal		= Vector3f::UnitZ();
	float SqueezeWeight = 0.0f;
	Vector3f Center		= Vector3f::Zero();
	float Distance2		= 0.0f;
};

// Spatial Hashmap
class PathVertexMap : public HashGrid<size_t, vcm_position_getter> {
	PR_CLASS_NON_COPYABLE(PathVertexMap);
public:
	using CheckFunction = bool (*)(const PathVertex&, const QuerySphere&, float&);

	inline PathVertexMap(const BoundingBox& bbox, float gridDelta, const std::vector<PathVertex>& vertices);
	inline virtual ~PathVertexMap();

	template <typename AccumFunction>
	inline SpectralBlob estimateSphere(const QuerySphere& sphere, const AccumFunction& accumFunc, size_t& found) const;

	template <typename AccumFunction>
	inline SpectralBlob estimateDome(const QuerySphere& sphere, const AccumFunction& accumFunc, size_t& found) const;

	template <typename AccumFunction>
	inline SpectralBlob estimate(const QuerySphere& sphere, const CheckFunction& checkFunc, const AccumFunction& accumFunc, size_t& found) const;

private:const std::vector<PathVertex>& mVertices;
};
} // namespace VCM
} // namespace PR

#include "PathVertexMap.inl"
