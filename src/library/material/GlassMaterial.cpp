#include "GlassMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

#include "BRDF.h"

namespace PR
{
	GlassMaterial::GlassMaterial() :
		Material(), 
		mCameraVisible(true), mSpecularitySpectrum(), mIndex(1), mFresnel(0)
	{
	}

	Spectrum GlassMaterial::specularity() const
	{
		return mSpecularitySpectrum;
	}

	void GlassMaterial::setSpecularity(const Spectrum& spec)
	{
		mSpecularitySpectrum = spec;
	}

	float GlassMaterial::index() const
	{
		return mIndex;
	}

	void GlassMaterial::setIndex(float f)
	{
		mIndex = f;
		const float tmp = (1 - mIndex) / (1 + mIndex);
		mFresnel = tmp * tmp;
	}

	bool GlassMaterial::isLight() const
	{
		return false;
	}

	void GlassMaterial::enableCameraVisibility(bool b)
	{
		mCameraVisible = b;
	}

	bool GlassMaterial::isCameraVisible() const
	{
		return mCameraVisible;
	}

	bool GlassMaterial::shouldIgnore_Simple(const Ray& in, RenderEntity* entity)
	{
		return !mCameraVisible && in.depth() == 0;
	}

	constexpr float NormalOffset = 0.0001f;
	void GlassMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
		float reflection = BRDF::fresnel_schlick(mFresnel, in.direction(), point.normal()) / 4;

		Spectrum spec;
		if (reflection > PM_EPSILON)
		{
			PM::vec3 reflect = BRDF::reflect(point.normal(), in.direction());
			Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(reflect, NormalOffset)), reflect, in.depth() + 1);

			FacePoint collisionPoint;
			if (renderer->shoot(ray, collisionPoint))
			{
				spec += reflection * ray.spectrum();
			}
		}

		if (1 - reflection > PM_EPSILON)
		{
			PM::vec3 refract = BRDF::refract(mIndex, point.normal(), in.direction());

			Ray ray(PM::pm_Add(point.vertex(), PM::pm_Scale(refract, NormalOffset)), refract, in.depth() + 1);

			FacePoint collisionPoint;
			if (renderer->shoot(ray, collisionPoint))
			{
				spec += (1 - reflection) * ray.spectrum();
			}
		}

		in.setSpectrum(spec);
	}
}