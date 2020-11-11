// IWYU pragma: private, include "photon/PhotonMap.h"
namespace PR {
namespace Photon {
PhotonMap::PhotonMap(const BoundingBox& bbox, float gridDelta)
	: HashGrid(bbox, gridDelta)
{
}

PhotonMap::~PhotonMap()
{
}

template <typename AccumFunction>
SpectralBlob PhotonMap::estimateSphere(const PhotonSphere& sphere, const AccumFunction& accumFunc, size_t& found) const
{
	static const auto estSphereSqueeze = [](const Photon& pht, const PhotonSphere& sph, float& dist2) {
		const Vector3f V = pht.Position - sph.Center;
		dist2			 = V.squaredNorm();
		const float d	 = V.dot(sph.Normal);
		const float r	 = sph.Distance2 * (1 - std::abs(d)) + sph.Distance2 * std::abs(d) * (1 - sph.SqueezeWeight);
		return dist2 <= r;
	};
	static const auto estSphereNoSqueeze = [](const Photon& pht, const PhotonSphere& sph, float& dist2) {
		const Vector3f V = pht.Position - sph.Center;
		dist2			 = V.squaredNorm();
		return dist2 <= sph.Distance2;
	};
	return estimate<AccumFunction>(
		sphere,
		sphere.SqueezeWeight > PR_EPSILON ? estSphereSqueeze : estSphereNoSqueeze,
		accumFunc, found);
}

template <typename AccumFunction>
SpectralBlob PhotonMap::estimateDome(const PhotonSphere& sphere, const AccumFunction& accumFunc, size_t& found) const
{
	static const auto estDomeSqueeze = [](const Photon& pht, const PhotonSphere& sph, float& dist2) {
		const Vector3f V = pht.Position - sph.Center;
		dist2			 = V.squaredNorm();
		const float d	 = V.dot(sph.Normal);
		const float k	 = pht.Direction.dot(sph.Normal);
		const float r	 = sph.Distance2 * (1 - std::abs(d)) + sph.Distance2 * std::abs(d) * (1 - sph.SqueezeWeight);
		return k > -PR_EPSILON && dist2 <= r;
	};
	static const auto estDomeNoSqueeze = [](const Photon& pht, const PhotonSphere& sph, float& dist2) {
		const Vector3f V = pht.Position - sph.Center;
		dist2			 = V.squaredNorm();
		const float k	 = pht.Direction.dot(sph.Normal);
		return k > -PR_EPSILON && dist2 <= sph.Distance2;
	};
	return estimate<AccumFunction>(
		sphere,
		sphere.SqueezeWeight > PR_EPSILON ? estDomeSqueeze : estDomeNoSqueeze,
		accumFunc, found);
}

template <typename AccumFunction>
SpectralBlob PhotonMap::estimate(const PhotonSphere& sphere,
								 const CheckFunction& checkFunc, const AccumFunction& accumFunc, size_t& found) const
{
	found			  = 0;
	SpectralBlob spec = SpectralBlob::Zero();

	search(sphere.Center, std::sqrt(sphere.Distance2), [&](const Photon& pht) {
		float dist2;
		if (checkFunc(pht, sphere, dist2)) { // Found a photon!
			found++;
			accumFunc(spec, pht, sphere, dist2);
		}
	});

	return spec;
}
} // namespace Photon
} // namespace PR
