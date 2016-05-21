#include "GlassMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

#include "texture/Texture1D.h"

#include "BRDF.h"

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

	Spectrum GlassMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li)
	{
		// TODO
		if (mSpecularity)
			return Li * mSpecularity->eval(point.uv());
		else
			return Spectrum();
	}

	float GlassMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		return 0;
	}

	float GlassMaterial::emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		const float NdotV = PM::pm_Dot3D(V, point.normal());
		const bool inside = NdotV > 0;

		const float ind = index(0);// TODO: Average?

		const PM::vec3 N = inside ? PM::pm_Negate(point.normal()) : point.normal();
		dir = BRDF::refract(inside ? ind : 1/ind, N, V);

		return 1;
	}

	float GlassMaterial::roughness(const FacePoint& point) const
	{
		return 0;
	}

	float GlassMaterial::pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		return 1;
	}
}