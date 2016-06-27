#include "GlassMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

#include "texture/Texture1D.h"

#include "BRDF.h"
#include "math/Reflection.h"
#include "math/Fresnel.h"

namespace PR
{
	GlassMaterial::GlassMaterial() :
		Material(), 
		mSpecularity(nullptr), mIndex(nullptr)
	{
	}

	Texture2D* GlassMaterial::specularity() const
	{
		return mSpecularity;
	}

	void GlassMaterial::setSpecularity(Texture2D* spec)
	{
		mSpecularity = spec;
	}

	float GlassMaterial::index(float lambda) const
	{
		if (mIndex)
			return mIndex->eval(lambda);
		else
			return 0;
	}

	Data1D* GlassMaterial::indexData() const
	{
		return mIndex;
	}

	void GlassMaterial::setIndexData(Data1D* data)
	{
		mIndex = data;
	}

	Spectrum GlassMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		if (mSpecularity)
		{
			return mSpecularity->eval(point.uv());
		}
		else
		{
			return Spectrum();
		}
	}

	float GlassMaterial::pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		return std::numeric_limits<float>::infinity();
	}

	PM::vec3 GlassMaterial::sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf)
	{
		const float NdotV = PM::pm_Dot3D(V, point.normal());

		const float ind = index(PM::pm_GetX(rnd));
		const float d = !point.isInside() ?
			Fresnel::dielectric(NdotV, 1, ind) : Fresnel::dielectric(NdotV, ind, 1);

		PM::vec3 dir;
		if (PM::pm_GetY(rnd) < d)
		{
			dir = reflect(NdotV, point.normal(), V);
		}
		else
		{
			dir = refract(!point.isInside() ? 1/ind : ind, NdotV, point.normal(), V);
		}

		pdf = std::numeric_limits<float>::infinity();
		return dir;
	}
}