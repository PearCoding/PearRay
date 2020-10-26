// IWYU pragma: private, include "photon/PhotonMap.h"
namespace PR {
namespace Photon {
PhotonMap::PhotonMap(const BoundingBox& bbox, float gridDelta)
	: mPhotons()
	, mStoredPhotons(0)
	, mGridDelta(gridDelta)
	, mInvGridDelta(1.0f / gridDelta)
	, mBoundingBox(bbox)
	, mGridX(std::max<uint32>(1, std::ceil(bbox.edge(0) * mInvGridDelta)))
	, mGridY(std::max<uint32>(1, std::ceil(bbox.edge(1) * mInvGridDelta)))
	, mGridZ(std::max<uint32>(1, std::ceil(bbox.edge(2) * mInvGridDelta)))
{
	PR_ASSERT(mGridDelta > PR_EPSILON, "Grid delta has to greater 0");
	PR_ASSERT(std::isfinite(mInvGridDelta), "Inverse of grid delta has to be valid");

	mPhotons.resize(mGridX * mGridY * mGridZ);
}

PhotonMap::~PhotonMap()
{
}

void PhotonMap::reset()
{
	mStoredPhotons = 0;
	for (auto& m : mPhotons)
		m.clear();
}

template <typename AccumFunction>
SpectralBlob PhotonMap::estimateSphere(const PhotonSphere& sphere, const AccumFunction& accumFunc, size_t& found) const
{
	static const auto estSphereSqueeze = [](const Photon& pht, const PhotonSphere& sph, float& dist2) {
		Vector3f V	  = Vector3f(pht.Position[0], pht.Position[1], pht.Position[2]) - sph.Center;
		dist2		  = V.squaredNorm();
		const float d = V.dot(sph.Normal);
		const float r = sph.Distance2 * (1 - std::abs(d)) + sph.Distance2 * std::abs(d) * (1 - sph.SqueezeWeight);
		return dist2 <= r;
	};
	static const auto estSphereNoSqueeze = [](const Photon& pht, const PhotonSphere& sph, float& dist2) {
		Vector3f V = Vector3f(pht.Position[0], pht.Position[1], pht.Position[2]) - sph.Center;
		dist2	   = V.squaredNorm();
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
		Vector3f V	  = Vector3f(pht.Position[0], pht.Position[1], pht.Position[2]) - sph.Center;
		dist2		  = V.squaredNorm();
		const float d = V.dot(sph.Normal);
		const float r = sph.Distance2 * (1 - std::abs(d)) + sph.Distance2 * std::abs(d) * (1 - sph.SqueezeWeight);
		return d <= -PR_EPSILON && dist2 <= r;
	};
	static const auto estDomeNoSqueeze = [](const Photon& pht, const PhotonSphere& sph, float& dist2) {
		Vector3f V	  = Vector3f(pht.Position[0], pht.Position[1], pht.Position[2]) - sph.Center;
		dist2		  = V.squaredNorm();
		const float d = V.dot(sph.Normal);
		return d <= -PR_EPSILON && dist2 <= sph.Distance2;
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
	found = 0;

	KeyCoord centerCoord = toCoords(sphere.Center(0), sphere.Center(1), sphere.Center(2));
	const int32 rad		 = std::max<int32>(0, (int32)std::ceil(std::sqrt(sphere.Distance2) * mInvGridDelta)) + 1;
	const int32 rad2	 = rad * rad;

	float dist2;

	SpectralBlob spec = SpectralBlob::Zero();
	for (int x = -rad; x <= rad; ++x) {
		for (int y = -rad; y <= rad; ++y) {
			if (x * x + y * y > rad2)
				continue;

			for (int z = -rad; z <= rad; ++z) {
				if (x * x + y * y + z * z > rad2)
					continue;

				const int nx = centerCoord.X + x;
				const int ny = centerCoord.Y + y;
				const int nz = centerCoord.Z + z;
				if (nx < 0 || nx >= (int)mGridX || ny < 0 || ny >= (int)mGridY || nz < 0 || nz >= (int)mGridZ)
					continue;

				const KeyCoord key = { (uint32)nx, (uint32)ny, (uint32)nz };
				for (const auto& pht : mPhotons[toIndex(key)]) {
					if (checkFunc(pht, sphere, dist2)) { // Found a photon!
						found++;
						accumFunc(spec, pht, sphere, dist2);
					}
				}
			}
		}
	}
	return spec;
}

void PhotonMap::store(const Photon& pht)
{
	if (!mBoundingBox.contains(pht.Position))
		return;

	mStoredPhotons++;
	const auto key = toCoords(pht.Position[0], pht.Position[1], pht.Position[2]);

	mPhotons[toIndex(key)].push_back(pht);
}

typename PhotonMap::KeyCoord PhotonMap::toCoords(float x, float y, float z) const
{
	return {
		static_cast<uint32>(std::floor((x - mBoundingBox.lowerBound()(0)) * mInvGridDelta)),
		static_cast<uint32>(std::floor((y - mBoundingBox.lowerBound()(1)) * mInvGridDelta)),
		static_cast<uint32>(std::floor((z - mBoundingBox.lowerBound()(2)) * mInvGridDelta))
	};
}

bool PhotonMap::KeyCoord::operator==(const KeyCoord& other) const
{
	return X == other.X && Y == other.Y && Z == other.Z;
}

inline size_t PhotonMap::toIndex(const KeyCoord& coords) const
{
	return coords.X + coords.Y * mGridX + coords.Z * mGridX * mGridY;
}
} // namespace Photon
} // namespace PR
