#pragma once

#include "config/TriangleOptions.h"
#include "math/SIMD.h"
#include "math/Transform.h"
#include "spectral/SpectralBlob.h"

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

	FloatingType MinT				 = FloatingType(PR_EPSILON);
	FloatingType MaxT				 = FloatingType(std::numeric_limits<float>::infinity());
	SpectralBlobBase<V> Weight		 = SpectralBlobBase<V>(V(0), V(0), V(0), V(0));
	FloatingType Time				 = FloatingType(0);
	IntegerType IterationDepth		 = IntegerType(0);
	IntegerType Flags				 = IntegerType(0);
	SpectralBlobBase<V> WavelengthNM = SpectralBlobBase<V>(V(0), V(0), V(0), V(0)); // Hero Quartett, first entry is hero wavelength
	IntegerType PixelIndex			 = IntegerType(0);

private:
	bool Cached = false;
#ifdef PR_RAY_REQUIRE_MOMENTUM
	Vector3t<V> Momentum_Cache;
#endif
#ifdef PR_RAY_REQUIRE_MAX_DIRECTION
	IntegerType MaxDirectionIndex_Cache;
#endif
public:
	RayPackageBase()							= default;
	RayPackageBase(const RayPackageBase& other) = default;
	RayPackageBase(RayPackageBase&& other)		= default;
	RayPackageBase& operator=(const RayPackageBase& other) = default;
	RayPackageBase& operator=(RayPackageBase&& other) = default;

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
#ifdef PR_RAY_REQUIRE_MOMENTUM
		PR_ASSERT(Cached, "Cache first!");
		return Momentum_Cache;
#else
		return Origin.cross(Direction);
#endif
	}

	inline IntegerType maxDirectionIndex() const
	{
#ifdef PR_RAY_REQUIRE_MAX_DIRECTION
		PR_ASSERT(Cached, "Cache first!");
		return MaxDirectionIndex_Cache;
#else
		return calcMaxDirectionIndex();
#endif
	}

	inline void cache()
	{
#ifdef PR_RAY_REQUIRE_MOMENTUM
		Momentum_Cache = Origin.cross(Direction);
#endif
#ifdef PR_RAY_REQUIRE_MAX_DIRECTION
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

	inline RayPackageBase<V> next(const Vector3t<V>& o, const Vector3t<V>& d, const Vector3t<V>& N,
								  const IntegerType& vis_flags, const V& minT, const V& maxT) const
	{
		RayPackageBase<V> other;
		other = *this;

		other.Origin	= Transform::safePosition(o, d, N);
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
	ray.Origin[0]	 = extract(i, package.Origin[0]);
	ray.Origin[1]	 = extract(i, package.Origin[1]);
	ray.Origin[2]	 = extract(i, package.Origin[2]);
	ray.Direction[0] = extract(i, package.Direction[0]);
	ray.Direction[1] = extract(i, package.Direction[1]);
	ray.Direction[2] = extract(i, package.Direction[2]);
	for (size_t k = 0; k < SPECTRAL_BLOB_SIZE; ++k)
		ray.Weight[k] = extract(i, package.Weight[k]);
	ray.MinT		   = extract(i, package.MinT);
	ray.MaxT		   = extract(i, package.MaxT);
	ray.Time		   = extract(i, package.Time);
	ray.IterationDepth = extract(i, package.IterationDepth);
	ray.Flags		   = extract(i, package.Flags);
	for (size_t k = 0; k < SPECTRAL_BLOB_SIZE; ++k)
		ray.WavelengthNM[k] = extract(i, package.WavelengthNM[k]);
	ray.PixelIndex = extract(i, package.PixelIndex);
	ray.cache();
	return ray;
}

inline void insertIntoRayPackage(uint32 i, RayPackage& package, const Ray& ray)
{
	package.Origin[0]	 = insert(i, package.Origin[0], ray.Origin[0]);
	package.Origin[1]	 = insert(i, package.Origin[1], ray.Origin[1]);
	package.Origin[2]	 = insert(i, package.Origin[2], ray.Origin[2]);
	package.Direction[0] = insert(i, package.Direction[0], ray.Direction[0]);
	package.Direction[1] = insert(i, package.Direction[1], ray.Direction[1]);
	package.Direction[2] = insert(i, package.Direction[2], ray.Direction[2]);
	for (size_t k = 0; k < SPECTRAL_BLOB_SIZE; ++k)
		package.Weight[k] = insert(i, package.Weight[k], ray.Weight[k]);
	package.MinT		   = insert(i, package.MinT, ray.MinT);
	package.MaxT		   = insert(i, package.MaxT, ray.MaxT);
	package.Time		   = insert(i, package.Time, ray.Time);
	package.IterationDepth = insert(i, package.IterationDepth, ray.IterationDepth);
	package.Flags		   = insert(i, package.Flags, ray.Flags);
	for (size_t k = 0; k < SPECTRAL_BLOB_SIZE; ++k)
		package.WavelengthNM[k] = insert(i, package.WavelengthNM[k], ray.WavelengthNM[k]);
	package.PixelIndex = insert(i, package.PixelIndex, ray.PixelIndex);
	package.cache();
}

} // namespace PR