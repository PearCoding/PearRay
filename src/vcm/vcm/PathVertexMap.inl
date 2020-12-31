// IWYU pragma: private, include "vcm/PathVertexMap.h"
namespace PR {
namespace VCM {
PathVertexMap::PathVertexMap(const BoundingBox& bbox, float gridDelta, const std::vector<PathVertex>& vertices)
	: HashGrid(bbox, gridDelta, vcm_position_getter<size_t>{ vertices })
	, mVertices(vertices)
{
}

PathVertexMap::~PathVertexMap()
{
}

template <typename AccumFunction>
SpectralBlob PathVertexMap::estimateSphere(const QuerySphere& sphere, const AccumFunction& accumFunc, size_t& found) const
{
	static const auto estSphereSqueeze = [](const PathVertex& pht, const QuerySphere& sph, float& dist2) {
		const Vector3f V = pht.IP.P - sph.Center;
		dist2			 = V.squaredNorm();
		const float d	 = V.dot(sph.Normal);
		const float r	 = sph.Distance2 * (1 - std::abs(d)) + sph.Distance2 * std::abs(d) * (1 - sph.SqueezeWeight);
		return dist2 <= r;
	};
	static const auto estSphereNoSqueeze = [](const PathVertex& pht, const QuerySphere& sph, float& dist2) {
		const Vector3f V = pht.IP.P - sph.Center;
		dist2			 = V.squaredNorm();
		return dist2 <= sph.Distance2;
	};

	if (sphere.SqueezeWeight > PR_EPSILON)
		return estimate<AccumFunction>(sphere, estSphereSqueeze, accumFunc, found);
	else
		return estimate<AccumFunction>(sphere, estSphereNoSqueeze, accumFunc, found);
}

template <typename AccumFunction>
SpectralBlob PathVertexMap::estimateDome(const QuerySphere& sphere, const AccumFunction& accumFunc, size_t& found) const
{
	static const auto estDomeSqueeze = [](const PathVertex& pht, const QuerySphere& sph, float& dist2) {
		const Vector3f V = pht.IP.P - sph.Center;
		dist2			 = V.squaredNorm();
		const float d	 = V.dot(sph.Normal);
		const float k	 = -pht.IP.Ray.Direction.dot(sph.Normal);
		const float r	 = sph.Distance2 * (1 - std::abs(d)) + sph.Distance2 * std::abs(d) * (1 - sph.SqueezeWeight);
		return k > -PR_EPSILON && dist2 <= r;
	};
	static const auto estDomeNoSqueeze = [](const PathVertex& pht, const QuerySphere& sph, float& dist2) {
		const Vector3f V = pht.IP.P - sph.Center;
		dist2			 = V.squaredNorm();
		const float k	 = -pht.IP.Ray.Direction.dot(sph.Normal);
		return k > -PR_EPSILON && dist2 <= sph.Distance2;
	};

	if (sphere.SqueezeWeight > PR_EPSILON)
		return estimate<AccumFunction>(sphere, estDomeSqueeze, accumFunc, found);
	else
		return estimate<AccumFunction>(sphere, estDomeNoSqueeze, accumFunc, found);
}

template <typename AccumFunction>
SpectralBlob PathVertexMap::estimate(const QuerySphere& sphere,
									 const CheckFunction& checkFunc, const AccumFunction& accumFunc, size_t& found) const
{
	found			  = 0;
	SpectralBlob spec = SpectralBlob::Zero();

	search<true>(sphere.Center, sphere.Distance2, [&](size_t id) {
		const PathVertex& pht = mVertices[id]; // Bad Hack?
		float dist2;
		if (checkFunc(pht, sphere, dist2)) { // Found a photon!
			found++;
			accumFunc(spec, pht, dist2);
		}
	});

	return spec;
}
} // namespace VCM
} // namespace PR
