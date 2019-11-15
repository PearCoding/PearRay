#pragma once

#include "math/Transform.h"
#include "spectral/ColorTriplet.h"

namespace PR {

enum RayFlags : uint32 {
	RF_Monochrome = 0x01,
	RF_Invalid	= 0x80
};

template <typename V>
struct RayPackageBase {
	typedef V FloatingType;
	typedef typename VectorTemplate<V>::uint32_t IntegerType;

	Vector3t<V> Origin	= Vector3t<V>(V(0), V(0), V(0));
	Vector3t<V> Direction = Vector3t<V>(V(0), V(0), V(0));

	FloatingType NdotL			= FloatingType(1);
	ColorTripletBase<V> Weight  = ColorTripletBase<V>(V(0), V(0), V(0));
	FloatingType Time			= FloatingType(0);
	IntegerType IterationDepth  = IntegerType(0);
	IntegerType Flags			= IntegerType(0);
	IntegerType WavelengthIndex = IntegerType(0);
	IntegerType PixelIndex		= IntegerType(0);

	RayPackageBase() = default;
	inline RayPackageBase(const Vector3t<V>& o, const Vector3t<V>& d,
						  const FloatingType& ndotl = FloatingType(1))
		: Origin(o)
		, Direction(d)
		, NdotL(ndotl)
	{
	}

	inline void normalize()
	{
		// We could use Eigen::...::normalize() but this has a check z > 0
		// we have to avoid.
		Direction /= Direction.norm();
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

	/* Advance with t, transform position with matrix and calculate new position to given other base. */
	inline V distanceTransformed(const V& t_local,
								 const Eigen::Matrix4f& local_to_global,
								 const RayPackageBase<V>& global_other) const
	{
		auto p  = this->t(t_local);
		auto p2 = Transform::apply(local_to_global, p); // TODO: Assume Affine?

		return (p2 - global_other.Origin).norm();
	}

	inline RayPackageBase<V> next(const Vector3t<V>& o, const Vector3t<V>& d,
								  const FloatingType& ndotl = FloatingType(1)) const
	{
		RayPackageBase<V> other;
		other = *this;

		other.Origin	= Transform::safePosition(o, d);
		other.Direction = d;
		other.NdotL		= ndotl;
		other.IterationDepth += IntegerType(1);

		return other;
	}
};

typedef RayPackageBase<vfloat> RayPackage;
typedef RayPackageBase<float> Ray;

} // namespace PR