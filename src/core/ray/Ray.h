#pragma once

#include "math/Transform.h"
#include "spectral/SpectralBlob.h"

namespace PR {

enum RayFlags : uint32 {
	// Matches EntityVisibilityFlags
	RF_Camera	  = 0x01,
	RF_Light	  = 0x02,
	RF_Bounce	  = 0x04,
	RF_Shadow	  = 0x08,
	RF_Monochrome = 0x10
};

struct PR_LIB_CORE Ray {
	Vector3f Origin	   = Vector3f::Zero();
	Vector3f Direction = Vector3f::Zero();

	float MinT				  = PR_EPSILON;
	float MaxT				  = PR_INF;
	SpectralBlob WavelengthNM = SpectralBlob::Zero(); // Hero Quartett, first entry is hero wavelength
	uint32 IterationDepth	  = 0;
	uint32 Flags			  = 0;
	uint32 PixelIndex		  = 0; // Tile local pixel index
	uint32 GroupID			  = 0; // Group to corresponding ray group if available

public:
	Ray()				  = default;
	Ray(const Ray& other) = default;
	Ray(Ray&& other)	  = default;
	Ray& operator=(const Ray& other) = default;
	Ray& operator=(Ray&& other) = default;

	inline Ray(const Vector3f& o, const Vector3f& d)
		: Origin(o)
		, Direction(d)
	{
	}

	inline void normalize()
	{
		Direction.normalize();
	}

	inline Ray transform(const Eigen::Ref<const Eigen::Matrix4f>& oM,
						 const Eigen::Ref<const Eigen::Matrix3f>& dM) const
	{
		Ray other;
		other = *this;

		other.Origin	= Transform::apply(oM, Origin);
		other.Direction = Transform::applyVector(dM, Direction);

		const float factor = other.Direction.norm();
		other.MinT		   = std::min(PR_EPSILON, MinT * factor);
		other.MaxT		   = MaxT * factor;

		other.normalize();

		return other;
	}

	inline Ray transformAffine(const Eigen::Ref<const Eigen::Matrix4f>& oM,
							   const Eigen::Ref<const Eigen::Matrix3f>& dM) const
	{
		Ray other;
		other = *this;

		other.Origin	= Transform::applyAffine(oM, Origin);
		other.Direction = Transform::applyVector(dM, Direction);

		const float factor = other.Direction.norm();
		other.MinT		   = std::min(PR_EPSILON, MinT * factor);
		other.MaxT		   = MaxT * factor;

		other.normalize();

		return other;
	}

	inline auto /*Vector3f*/ t(float t) const
	{
		return Origin + t * Direction;
	}

	/* Advance direction with t, transform displacement with direction matrix and calculate norm of result. */
	inline float transformDistance(float t_local,
								   const Eigen::Ref<const Eigen::Matrix3f>& directionMatrix) const
	{
		const Vector3f dt  = t_local * Direction;
		const Vector3f dt2 = Transform::applyVector(directionMatrix, dt);

		return dt2.norm();
	}

	inline Ray next(const Vector3f& o, const Vector3f& d, const Vector3f& N,
					uint32 vis_flags, float minT, float maxT) const
	{
		Ray other;
		other = *this;

		other.Origin	= Transform::safePosition(o, d, N);
		other.Direction = d;
		other.IterationDepth += 1;
		other.MinT = minT;
		other.MaxT = maxT;
		other.Flags |= vis_flags;

		return other;
	}

	inline bool isInsideRange(float t) const
	{
		return t >= MinT && t <= MaxT;
	}
};

} // namespace PR