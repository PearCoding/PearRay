#include "DebugBoundingBoxMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

namespace PR
{
	DebugBoundingBoxMaterial::DebugBoundingBoxMaterial() :
		Material()
	{
	}

	void DebugBoundingBoxMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li,
		Spectrum& diff, Spectrum& spec)
	{
		//Ray ray = in;// Don't increase depth!
		//ray.setStartPosition(point.vertex());

		//FacePoint pt;
		//renderer->shoot(ray, pt, entity);

		spec = Li*(1 - mDensity) + mColor*mDensity;
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
	
	float DebugBoundingBoxMaterial::roughness() const
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
}