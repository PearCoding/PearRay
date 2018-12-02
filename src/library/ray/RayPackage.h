#pragma once

#include "math/SIMD.h"

namespace PR {

template <typename V, typename IV>
struct PR_LIB_INLINE RayPackageBase {
	typedef V FloatingType;
	typedef IV IntegerType;

	V Origin[3];
	V Direction[3];
	V InvDirection[3];

	V Weight;
	V Time;
	IV Depth;
	IV Flags;
	IV WavelengthIndex;
	IV PixelIndex;

	RayPackageBase() = default;
	inline RayPackageBase(const V& ox, const V& oy, const V& oz,
						  const V& dx, const V& dy, const V& dz)
		: Origin{ ox, oy, oz }
		, Direction{ dx, dy, dz }
	{
		setupInverse();
	}

	inline void setupInverse()
	{
		for (int i = 0; i < 3; ++i)
			InvDirection[i] = 1 / Direction[i];
	}

	inline void normalize()
	{
		V n = 1 / (Direction[0] * Direction[0] + Direction[1] * Direction[1] + Direction[2] * Direction[2]);

		for (int i = 0; i < 3; ++i)
			Direction[i] = Direction[i] * n;
	}

	inline RayPackageBase<V, IV> transform(const Eigen::Matrix4f& oM, const Eigen::Matrix3f& dM) const
	{
		RayPackageBase<V, IV> other;
		transformV(oM,
				   Origin[0], Origin[1], Origin[2],
				   other.Origin[0], other.Origin[1], other.Origin[2]);
		transformV(dM,
				   Direction[0], Direction[1], Direction[2],
				   other.Direction[0], other.Direction[1], other.Direction[2]);

		other.normalize();
		other.setupInverse();

		return other;
	}

	inline void t(const V& t, V& px, V& py, V& pz) const
	{
		px = Origin[0] + t * Direction[0];
		py = Origin[1] + t * Direction[1];
		pz = Origin[2] + t * Direction[2];
	}

	/* Advance with t, transform position with matrix and calculate new position to given other base. */
	inline V distance_transformed(const V& t_local,
								  const Eigen::Matrix4f& local_to_global,
								  const RayPackageBase<V, IV>& global_other) const
	{
		V px, py, pz;
		this->t(t_local, px, py, pz);

		V px2, py2, pz2;
		transformV(local_to_global,
				   px, py, pz,
				   px2, py2, pz2);

		px = px2 - global_other.Origin[0];
		py = py2 - global_other.Origin[1];
		pz = pz2 - global_other.Origin[2];

		return magnitudeV(px, py, pz);
	}
};

typedef RayPackageBase<vfloat, vuint32> RayPackage;
typedef RayPackageBase<float, uint32> Ray;

template <typename T>
struct RayTemplate : public VectorTemplate<typename T::FloatingType> {
};

} // namespace PR