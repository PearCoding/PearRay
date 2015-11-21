#include "DebugBoundingBoxMaterial.h"
#include "ray/Ray.h"
#include "renderer/Renderer.h"
#include "geometry/FacePoint.h"

namespace PR
{
	DebugBoundingBoxMaterial::DebugBoundingBoxMaterial() :
		Material()
	{
	}

	void DebugBoundingBoxMaterial::apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer)
	{
		Ray ray = in;// Don't increase depth!
		ray.setStartPosition(point.vertex());

		FacePoint pt;
		renderer->shoot(ray, pt, entity);

		in.setSpectrum(ray.spectrum()*(1 - mDensity) + mColor*mDensity);
	}

	bool DebugBoundingBoxMaterial::isLight() const
	{
		return false;
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