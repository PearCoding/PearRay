// IWYU pragma: private, include "photon/PhotonMap.h"
namespace PR {
namespace Photon {
PhotonMap::PhotonMap(float gridDelta)
	: mPhotons()
	, mStoredPhotons(0)
	, mGridDelta(gridDelta)
	, mInvGridDelta(1.0f / gridDelta)
{
	PR_ASSERT(mGridDelta > PR_EPSILON, "Grid delta has to greater 0");
	PR_ASSERT(std::isfinite(mInvGridDelta), "Inverse of grid delta has to be valid");
}

PhotonMap::~PhotonMap()
{
}

void PhotonMap::reset()
{
	mStoredPhotons = 0;
	mPhotons.clear();
}

template <typename AccumFunction>
SpectralBlob PhotonMap::estimateSphere(const PhotonSphere& sphere, const AccumFunction& accumFunc, size_t& found) const
{
	return estimate<AccumFunction>(
		sphere,
		[](const Photon& pht, const PhotonSphere& sph, float& dist2) {
			Vector3f V	  = Vector3f(pht.Position[0], pht.Position[1], pht.Position[2]) - sph.Center;
			dist2		  = V.squaredNorm();
			const float d = V.dot(sph.Normal);
			const float r = sph.Distance2 * (1 - std::abs(d)) + sph.Distance2 * std::abs(d) * (1 - sph.SqueezeWeight);
			return dist2 <= r;
		},
		accumFunc, found);
}

template <typename AccumFunction>
SpectralBlob PhotonMap::estimateDome(const PhotonSphere& sphere, const AccumFunction& accumFunc, size_t& found) const
{
	return estimate<AccumFunction>(
		sphere,
		[](const Photon& pht, const PhotonSphere& sph, float& dist2) {
			Vector3f V	  = Vector3f(pht.Position[0], pht.Position[1], pht.Position[2]) - sph.Center;
			dist2		  = V.squaredNorm();
			const float d = V.dot(sph.Normal);
			const float r = sph.Distance2 * (1 - std::abs(d)) + sph.Distance2 * std::abs(d) * (1 - sph.SqueezeWeight);
			return d <= -PR_EPSILON && dist2 <= r;
		},
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

				KeyCoord key = {
					centerCoord.X + x,
					centerCoord.Y + y,
					centerCoord.Z + z
				};

				const auto range = mPhotons.equal_range(key);
				for (auto it = range.first; it != range.second; ++it) {
					if (checkFunc(it->second, sphere, dist2)) { // Found a photon!
						found++;
						accumFunc(spec, it->second, sphere, dist2);
					}
				}
			}
		}
	}
	return spec;
}

void PhotonMap::store(const Photon& pht)
{
	mStoredPhotons++;
	const auto key = toCoords(pht.Position[0], pht.Position[1], pht.Position[2]);

	mPhotons.insert(std::make_pair(key, pht));

	mBox.combine(Vector3f(pht.Position[0], pht.Position[1], pht.Position[2]));
}

typename PhotonMap::KeyCoord PhotonMap::toCoords(float x, float y, float z) const
{
	return {
		static_cast<int32>(std::floor(x * mInvGridDelta)),
		static_cast<int32>(std::floor(y * mInvGridDelta)),
		static_cast<int32>(std::floor(z * mInvGridDelta))
	};
}

bool PhotonMap::KeyCoord::operator==(const KeyCoord& other) const
{
	return X == other.X && Y == other.Y && Z == other.Z;
}

size_t PhotonMap::hash_compare::hash(const KeyCoord& coord)
{
	size_t seed = hash_union(coord.X, coord.Y);
	hash_combine(seed, coord.Z);
	return seed;
}

bool PhotonMap::hash_compare::equal(const KeyCoord& k1, const KeyCoord& k2)
{
	return k1 == k2;
}
} // namespace Photon
} // namespace PR
