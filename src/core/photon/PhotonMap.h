#pragma once

#include "geometry/BoundingBox.h"
#include "math/Generator.h"

#include "math/Hash.h"
#include "photon/Photon.h"

#include <atomic>
#include <tbb/concurrent_hash_map.h>
#include <vector>

namespace PR {
namespace Photon {
struct PhotonSphere // Setup for the estimation query
{
	uint64 MaxPhotons;
	Vector3f Normal;
	float SqueezeWeight;
	Vector3f Center;
	float Distance2;
};

// Spatial Hashmap
class PhotonMap {
	PR_CLASS_NON_COPYABLE(PhotonMap);

public:
	using CheckFunction = bool (*)(const Photon&, const PhotonSphere&, float&);

	inline explicit PhotonMap(float gridDelta);
	inline ~PhotonMap();

	inline void reset();

	inline bool isEmpty() const { return mStoredPhotons == 0; }
	inline uint64 storedPhotons() const { return mStoredPhotons; }

	template <typename AccumFunction>
	inline SpectralBlob estimateSphere(const PhotonSphere& sphere, const AccumFunction& accumFunc, size_t& found) const;

	template <typename AccumFunction>
	inline SpectralBlob estimateDome(const PhotonSphere& sphere, const AccumFunction& accumFunc, size_t& found) const;

	template <typename AccumFunction>
	inline SpectralBlob estimate(const PhotonSphere& sphere, const CheckFunction& checkFunc, const AccumFunction& accumFunc, size_t& found) const;

	inline void store(const Photon& point);

private:
	struct KeyCoord {
		int32 X, Y, Z;

		inline bool operator==(const KeyCoord& other) const;
	};

	inline KeyCoord toCoords(float x, float y, float z) const;

	struct hash_compare {
		inline static size_t hash(const KeyCoord&);
		inline static bool equal(const KeyCoord& k1, const KeyCoord& k2);
	};

	using Map = tbb::concurrent_hash_map<KeyCoord, std::vector<Photon>, hash_compare>;
	Map mPhotons;

	std::atomic<uint64> mStoredPhotons;
	const float mGridDelta;
	const float mInvGridDelta;

	BoundingBox mBox;
};
} // namespace Photon
} // namespace PR

#include "PhotonMap.inl"
