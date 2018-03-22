#pragma once

#include "geometry/BoundingBox.h"
#include "math/Generator.h"
#include "spectral/Spectrum.h"

#include "photon/Photon.h"

#include <atomic>
#include <tbb/concurrent_hash_map.h>
#include <vector>

//#define PR_USE_APPROX_PHOTON_MAP

namespace PR {
namespace Photon {
struct PhotonSphere // Setup for the estimation query
{
	uint64 MaxPhotons;
	Eigen::Vector3f Normal;
	float SqueezeWeight;
	Eigen::Vector3f Center;
	float Distance2;
};

// Spatial Hashmap
class PhotonMap {
	PR_CLASS_NON_COPYABLE(PhotonMap);

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef bool (*CheckFunction)(const Photon&, const PhotonSphere&, float&);

	inline explicit PhotonMap(float gridDelta);
	inline ~PhotonMap();

	inline void reset();

	inline bool isEmpty() const { return mStoredPhotons == 0; }
	inline uint64 storedPhotons() const { return mStoredPhotons; }

	template <typename AccumFunction>
	inline void estimateSphere(Spectrum& spec, const PhotonSphere& sphere, AccumFunction accumFunc, size_t& found) const;

	template <typename AccumFunction>
	inline void estimateDome(Spectrum& spec, const PhotonSphere& sphere, AccumFunction accumFunc, size_t& found) const;

	template <typename AccumFunction>
	inline void estimate(Spectrum& spec, const PhotonSphere& sphere, CheckFunction checkFunc, AccumFunction accumFunc, size_t& found) const;

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

#ifdef PR_USE_APPROX_PHOTON_MAP
	struct ApproxPhoton {
		Photon Approximation;
		uint32 Count;
		inline ApproxPhoton()
			: Count(0)
		{
		}
	};
	typedef tbb::concurrent_hash_map<KeyCoord, ApproxPhoton, hash_compare> Map;
#else
	typedef tbb::concurrent_hash_map<KeyCoord, std::vector<Photon>, hash_compare> Map;
#endif
	Map mPhotons;

	std::atomic<uint64> mStoredPhotons;
	const float mGridDelta;

	// Cache:
	float mInvGridDelta;

	BoundingBox mBox;
};
}
}

#include "PhotonMap.inl"
