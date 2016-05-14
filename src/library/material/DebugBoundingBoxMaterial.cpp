#include "DebugBoundingBoxMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

namespace PR
{
	DebugBoundingBoxMaterial::DebugBoundingBoxMaterial() :
		Material(), mDensity(0.5f)
	{
	}

	Spectrum DebugBoundingBoxMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li)
	{
		return Li*(1 - mDensity) + mColor*mDensity;
	}
	
	float DebugBoundingBoxMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		return 0;
	}

	float DebugBoundingBoxMaterial::emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		dir = V;
		return 1;
	}
	
	float DebugBoundingBoxMaterial::roughness(const FacePoint& point) const
	{
		return 0;
	}

	void DebugBoundingBoxMaterial::setColor(const Spectrum& spec)
	{
		mColor = spec;
	}

	Spectrum DebugBoundingBoxMaterial::color() const
	{
		return mColor;
	}

	void DebugBoundingBoxMaterial::setDensity(float f)
	{
		mDensity = f;
	}

	float DebugBoundingBoxMaterial::density() const
	{
		return mDensity;
	}

	float DebugBoundingBoxMaterial::pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		return 1;
	}
}