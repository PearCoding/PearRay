#pragma once

#include "math/SIMD.h"
#include "math/Transform.h"
#include "spectral/ColorTriplet.h"

#define PR_USE_RAY_CACHE

namespace PR {

enum RayFlags : uint32 {
	// Matches EntityVisibilityFlags
	RF_Camera	  = 0x01,
	RF_Light	  = 0x02,
	RF_Bounce	  = 0x04,
	RF_Shadow	  = 0x08,
	RF_Monochrome = 0x10,

	RF_VisibilityMask = RF_Camera | RF_Light | RF_Bounce | RF_Shadow
};

template <typename V>
struct RayPackageBase {
	using FloatingType = V;
	using IntegerType  = typename VectorTemplate<V>::uint32_t;
	using BoolType	   = typename VectorTemplate<V>::bool_t;

	Vector3t<V> Origin	  = Vector3t<V>(V(0), V(0), V(0));
	Vector3t<V> Direction = Vector3t<V>(V(0), V(0), V(0));

	FloatingType MinT			= FloatingType(PR_EPSILON);
	FloatingType MaxT			= FloatingType(std::numeric_limits<float>::infinity());
	ColorTripletBase<V> Weight	= ColorTripletBase<V>(V(0), V(0), V(0));
	FloatingType Time			= FloatingType(0);
	IntegerType IterationDepth	= IntegerType(0);
	IntegerType Flags			= IntegerType(0);
	IntegerType WavelengthIndex = IntegerType(0);
	IntegerType PixelIndex		= IntegerType(0);

private:
	bool Cached = false;
#ifdef PR_USE_RAY_CACHE
	Vector3t<V> Momentum_Cache;
	IntegerType MaxDirectionIndex_Cache;
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

	inline RayPackageBase<V> transform(const Eigen::Ref<const Eigen::Matrix4f>& oM,
									   const Eigen::Ref<const Eigen::Matrix3f>& dM) const
	{
		RayPackageBase<V> other;
		other = *this;

		other.Origin	= Transform::apply(oM, Origin);
		other.Direction = Transform::applyVector(dM, Direction);

		const V factor = other.Direction.norm();
		other.MinT	   = vmin(V(PR_EPSILON), MinT * factor);
		other.MaxT	   = MaxT * factor;

		other.normalize();

		return other;
	}

	inline RayPackageBase<V> transformAffine(const Eigen::Ref<const Eigen::Matrix4f>& oM,
											 const Eigen::Ref<const Eigen::Matrix3f>& dM) const
	{
		RayPackageBase<V> other;
		other = *this;

		other.Origin	= Transform::applyAffine(oM, Origin);
		other.Direction = Transform::applyVector(dM, Direction);

		const V factor = other.Direction.norm();
		other.MinT	   = vmin(V(PR_EPSILON), MinT * factor);
		other.MaxT	   = MaxT * factor;

		other.normalize();

		return other;
	}

	inline Vector3t<V> t(const V& t) const
	{
		return Origin + t * Direction;
	}

	inline Vector3t<V> momentum() const
	{
#ifdef PR_USE_RAY_CACHE
		PR_ASSERT(Cached, "Cache first!");
		return Momentum_Cache;
#else
		return Origin.cross(Direction);
#endif
	}

	inline IntegerType maxDirectionIndex() const
	{
#ifdef PR_USE_RAY_CACHE
		PR_ASSERT(Cached, "Cache first!");
		return MaxDirectionIndex_Cache;
#else
		return calcMaxDirectionIndex();
#endif
	}

	inline void cache()
	{
#ifdef PR_USE_RAY_CACHE
		Momentum_Cache			= Origin.cross(Direction);
		MaxDirectionIndex_Cache = calcMaxDirectionIndex();
#endif

		Cached = true;
	}

	/* Advance direction with t, transform displacement with direction matrix and calculate norm of result. */
	inline V transformDistance(const V& t_local,
							   const Eigen::Ref<const Eigen::Matrix3f>& directionMatrix) const
	{
		const Vector3t<V> dt  = t_local * Direction;
		const Vector3t<V> dt2 = Transform::applyVector(directionMatrix, dt);

		return dt2.norm();
	}

	inline RayPackageBase<V> next(const Vector3t<V>& o, const Vector3t<V>& d, const IntegerType& vis_flags, const V& minT, const V& maxT) const
	{
		RayPackageBase<V> other;
		other = *this;

		other.Origin	= Transform::safePosition(o, d);
		other.Direction = d;
		other.IterationDepth += IntegerType(1);
		other.MinT	= minT;
		other.MaxT	= maxT;
		other.Flags = vis_flags | (other.Flags & IntegerType(~RF_VisibilityMask));

		other.cache();

		return other;
	}

	inline BoolType isInsideRange(const V& t) const
	{
		return b_and(t >= MinT, t <= MaxT);
	}

private:
	inline IntegerType calcMaxDirectionIndex() const
	{
		FloatingType d0 = abs(Direction[0]);
		FloatingType d1 = abs(Direction[1]);
		FloatingType d2 = abs(Direction[2]);

		FloatingType maxD = blend(d0, d1, d0 > d1);
		IntegerType max	  = blend(IntegerType(0), IntegerType(1), d0 > d1);
		max				  = blend(max, IntegerType(2), maxD > d2);
		return max;
	}
};

using RayPackage = RayPackageBase<vfloat>;
using Ray		 = RayPackageBase<float>;

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
	ray.MinT			= extract(i, package.MinT);
	ray.MaxT			= extract(i, package.MaxT);
	ray.Time			= extract(i, package.Time);
	ray.IterationDepth	= extract(i, package.IterationDepth);
	ray.Flags			= extract(i, package.Flags);
	ray.WavelengthIndex = extract(i, package.WavelengthIndex);
	ray.PixelIndex		= extract(i, package.PixelIndex);
	ray.cache();
	return ray;
}

} // namespace PR