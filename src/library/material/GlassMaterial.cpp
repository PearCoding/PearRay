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
		mSpecularitySpectrum(), mIndex(1), mFresnel(0)
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

	Spectrum GlassMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li)
	{
		// TODO
		return Li * mSpecularitySpectrum;
	}

	float GlassMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		const float reflection = BRDF::fresnel_schlick(mFresnel, V, point.normal()) / 4;

		if (reflection > PM_EPSILON)
		{
			dir = BRDF::reflect(point.normal(), V);
			return reflection;
		}
		return 0;
	}

	float GlassMaterial::emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		const float reflection = BRDF::fresnel_schlick(mFresnel, V, point.normal()) / 4;
		const float NdotV = PM::pm_Dot3D(V, point.normal());
		const bool inside = NdotV < 0;

		if (1 - reflection > PM_EPSILON)
		{
			dir = BRDF::refract(inside ? mIndex : 1, inside ? 1 : mIndex, point.normal(), V);
			return 1 - reflection;
		}

		return 0;
	}

	float GlassMaterial::roughness() const
	{
		return 0;
	}
}