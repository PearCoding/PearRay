#include "MirrorMaterial.h"
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
	MirrorMaterial::MirrorMaterial() :
		Material(), 
		mSpecularity(nullptr), mIndex(nullptr)
	{
	}

	Texture2D* MirrorMaterial::specularity() const
	{
		return mSpecularity;
	}

	void MirrorMaterial::setSpecularity(Texture2D* spec)
	{
		mSpecularity = spec;
	}

	float MirrorMaterial::index(float lambda) const
	{
		if (mIndex)
			return mIndex->eval(lambda);
		else
			return 0;
	}

	Data1D* MirrorMaterial::indexData() const
	{
		return mIndex;
	}

	void MirrorMaterial::setIndexData(Data1D* data)
	{
		mIndex = data;
	}

	Spectrum MirrorMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		if (mSpecularity)
			return mSpecularity->eval(point.uv());
		else
			return Spectrum();
	}

	float MirrorMaterial::pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		return std::numeric_limits<float>::infinity();
	}

	PM::vec3 MirrorMaterial::sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf)
	{
		const float ind = index(PM::pm_GetX(rnd));
		auto dir = Reflection::reflect(PM::pm_Dot3D(point.normal(), V), point.normal(), V);

		pdf = std::numeric_limits<float>::infinity();
		return dir;
	}
}