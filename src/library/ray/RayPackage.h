#pragma once

#include "math/SIMD.h"
#include "math/Transform.h"
#include "spectral/ColorTriplet.h"

#define PR_RAY_CACHE_MOMENTUM

namespace PR {

enum RayFlags : uint32 {
	// Matches EntityVisibilityFlags
	RF_Camera	 = 0x01,
	RF_Light	  = 0x02,
	RF_Bounce	 = 0x04,
	RF_Shadow	 = 0x08,
	RF_Monochrome = 0x10
};

template <typename V>
struct RayPackageBase {
	typedef V FloatingType;
	typedef typename VectorTemplate<V>::uint32_t IntegerType;

	Vector3t<V> Origin	= Vector3t<V>(V(0), V(0), V(0));
	Vector3t<V> Direction = Vector3t<V>(V(0), V(0), V(0));

	ColorTripletBase<V> Weight  = ColorTripletBase<V>(V(0), V(0), V(0));
	FloatingType Time			= FloatingType(0);
	IntegerType IterationDepth  = IntegerType(0);
	IntegerType Flags			= IntegerType(0);
	IntegerType WavelengthIndex = IntegerType(0);
	IntegerType PixelIndex		= IntegerType(0);

private:
	bool Cached = false;
#ifdef PR_RAY_CACHE_MOMENTUM
	Vector3t<V> Momentum_Cache;
#endif
public:
	RayPackageBase() = default;
	inline RayPackageBase(const Vector3t<V>& o, const Vector3t<V>& d)
		: Origin(o)
		, Direction(d)
	{
		cache();
	}

	inline void normalize()
	{
		// We could use Eigen::...::normalize() but this has a check z > 0
		// we have to avoid.
		Direction /= Direction.norm();
		cache();
	}

	inline RayPackageBase<V> transform(const Eigen::Matrix4f& oM,
									   const Eigen::Matrix3f& dM) const
	{
		RayPackageBase<V> other;
		other = *this;

		other.Origin	= Transform::apply(oM, Origin);
		other.Direction = Transform::apply(dM, Direction);

		other.normalize();

		return other;
	}

	inline RayPackageBase<V> transformAffine(const Eigen::Matrix4f& oM,
											 const Eigen::Matrix3f& dM) const
	{
		RayPackageBase<V> other;
		other = *this;

		other.Origin	= Transform::applyAffine(oM, Origin);
		other.Direction = Transform::apply(dM, Direction);

		other.normalize();

		return other;
	}

	inline Vector3t<V> t(const V& t) const
	{
		return Origin + t * Direction;
	}

	inline Vector3t<V> momentum() const
	{
#ifdef PR_RAY_CACHE_MOMENTUM
		PR_ASSERT(Cached, "Cache first!");
		return Momentum_Cache;
#else
		return Origin.cross(Direction);
#endif
	}

	inline void cache()
	{
#ifdef PR_RAY_CACHE_MOMENTUM
		Momentum_Cache = Origin.cross(Direction);
#endif

		Cached = true;
	}

	/* Advance with t, transform position with matrix and calculate new position to given other base. */
	inline V distanceTransformed(const V& t_local,
								 const Eigen::Matrix4f& local_to_global,
								 const RayPackageBase<V>& global_other) const
	{
		auto p  = this->t(t_local);
		auto p2 = Transform::applyAffine(local_to_global, p);

		return (p2 - global_other.Origin).norm();
	}

	inline RayPackageBase<V> next(const Vector3t<V>& o, const Vector3t<V>& d) const
	{
		RayPackageBase<V> other;
		other = *this;

		other.Origin	= Transform::safePosition(o, d);
		other.Direction = d;
		other.IterationDepth += IntegerType(1);

		other.cache();

		return other;
	}
};

typedef RayPackageBase<vfloat> RayPackage;
typedef RayPackageBase<float> Ray;

inline Ray extractFromRayPackage(uint32 i, const RayPackage& package)
{
	Ray ray;
	ray.Origin[0]		= extract(i, package.Origin[0]);
	ray.Origin[1]		= extract(i, package.Origin[1]);
	ray.Origin[2]		= extract(i, package.Origin[2]);
	ray.Direction[0]	= extract(i, package.Direction[0]);
	ray.Direction[1]	= extract(i, package.Direction[1]);
	ray.Direction[2]	= extract(i, package.Direction[2]);
	ray.Weight[0]		= extract(i, package.Weight[0]);
	ray.Weight[1]		= extract(i, package.Weight[1]);
	ray.Weight[2]		= extract(i, package.Weight[2]);
	ray.Time			= extract(i, package.Time);
	ray.IterationDepth  = extract(i, package.IterationDepth);
	ray.Flags			= extract(i, package.Flags);
	ray.WavelengthIndex = extract(i, package.WavelengthIndex);
	ray.PixelIndex		= extract(i, package.PixelIndex);
	ray.cache();
	return ray;
}

} // namespace PR