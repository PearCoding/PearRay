#pragma once

#include "Enum.h"
#include "math/Transform.h"
#include "spectral/SpectralBlob.h"

namespace PR {

enum class RayFlag : uint32 {
	Camera	   = 0x01, // RayFlags::Camera and RayFlags::Light are exclusive
	Light	   = 0x02,
	Bounce	   = 0x04, // Bounce and RayFlags::Shadow are exclusive, if none of them is set its a primary ray
	Shadow	   = 0x08,
	Monochrome = 0x10,

	SourceMask = Camera | Light,
	TypeMask   = Bounce | Shadow,
};
PR_MAKE_FLAGS(RayFlag, RayFlags)

struct PR_LIB_CORE Ray {
	Vector3f Origin	   = Vector3f::Zero();
	Vector3f Direction = Vector3f::Zero();

	float MinT				  = PR_EPSILON;
	float MaxT				  = PR_INF;
	SpectralBlob WavelengthNM = SpectralBlob::Zero(); // Hero Quartett, first entry is hero wavelength
	uint32 IterationDepth	  = 0;
	RayFlags Flags			  = 0;
	uint32 PixelIndex		  = 0;			   // Global pixel index
	uint32 GroupID			  = PR_INVALID_ID; // Points to corresponding ray group if available

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

	inline Ray next(const Vector3f& o, const Vector3f& d,
					RayFlags vis_flags, float minT, float maxT) const
	{
		Ray other;
		other = *this;

		other.Origin	= o;
		other.Direction = d;
		other.IterationDepth += 1;
		other.MinT = minT;
		other.MaxT = maxT;
		other.Flags |= vis_flags;

		return other;
	}

	inline Ray next(const Vector3f& o, const Vector3f& d, const Vector3f& N,
					RayFlags vis_flags, float minT, float maxT) const
	{
		return next(Transform::safePosition(o, d, N), d, vis_flags, minT, maxT);
	}

	inline bool isInsideRange(float t) const
	{
		return t >= MinT && t <= MaxT;
	}
};

} // namespace PR