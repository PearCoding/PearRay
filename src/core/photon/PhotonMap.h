#pragma once

#include "container/HashGrid.h"
#include "photon/Photon.h"

namespace PR {
template <>
struct position_getter<Photon::Photon> {
	inline Vector3f operator()(const Photon::Photon& pht) const { return pht.Position; }
};

namespace Photon {

struct PhotonSphere // Setup for the estimation query
{
	Vector3f Normal		= Vector3f::UnitZ();
	float SqueezeWeight = 0.0f;
	Vector3f Center		= Vector3f::Zero();
	float Distance2		= 0.0f;
};

// Spatial Hashmap
class PhotonMap : public HashGrid<Photon> {
	PR_CLASS_NON_COPYABLE(PhotonMap);

public:
	using CheckFunction = bool (*)(const Photon&, const PhotonSphere&, float&);

	inline PhotonMap(const BoundingBox& bbox, float gridDelta);
	inline virtual ~PhotonMap();

	template <typename AccumFunction>
	inline SpectralBlob estimateSphere(const PhotonSphere& sphere, const AccumFunction& accumFunc, size_t& found) const;

	template <typename AccumFunction>
	inline SpectralBlob estimateDome(const PhotonSphere& sphere, const AccumFunction& accumFunc, size_t& found) const;

	template <typename AccumFunction>
	inline SpectralBlob estimate(const PhotonSphere& sphere, const CheckFunction& checkFunc, const AccumFunction& accumFunc, size_t& found) const;
};
} // namespace Photon
} // namespace PR

#include "PhotonMap.inl"
