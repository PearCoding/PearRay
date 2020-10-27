#pragma once

#include "geometry/BoundingBox.h"
#include "math/Generator.h"

#include "math/Hash.h"
#include "photon/Photon.h"

#include <atomic>
#include <tbb/concurrent_vector.h>
#include <vector>

namespace PR {
namespace Photon {
struct PhotonSphere // Setup for the estimation query
{
	Vector3f Normal		= Vector3f::UnitZ();
	float SqueezeWeight = 0.0f;
	Vector3f Center		= Vector3f::Zero();
	float Distance2		= 0.0f;
};

// Spatial Hashmap
class PhotonMap {
	PR_CLASS_NON_COPYABLE(PhotonMap);

public:
	using CheckFunction = bool (*)(const Photon&, const PhotonSphere&, float&);

	inline explicit PhotonMap(const BoundingBox& bbox, float gridDelta);
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
	inline void storeUnsafe(const Photon& point); // Do not check for the boundary

private:
	struct KeyCoord {
		uint32 X, Y, Z;

		inline bool operator==(const KeyCoord& other) const;
	};

	inline KeyCoord toCoords(float x, float y, float z) const;
	inline size_t toIndex(const KeyCoord& coords) const;

	using Map = std::vector<tbb::concurrent_vector<Photon>>;
	Map mPhotons;

	std::atomic<uint64> mStoredPhotons;
	const float mGridDelta;
	const float mInvGridDelta;
	const BoundingBox mBoundingBox;
	const uint32 mGridX;
	const uint32 mGridY;
	const uint32 mGridZ;
};
} // namespace Photon
} // namespace PR

#include "PhotonMap.inl"
