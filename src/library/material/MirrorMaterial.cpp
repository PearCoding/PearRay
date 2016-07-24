#include "MirrorMaterial.h"
#include "ray/Ray.h"
#include "shader/SamplePoint.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

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

	SpectralShaderOutput* MirrorMaterial::specularity() const
	{
		return mSpecularity;
	}

	void MirrorMaterial::setSpecularity(SpectralShaderOutput* spec)
	{
		mSpecularity = spec;
	}

	SpectralShaderOutput* MirrorMaterial::indexData() const
	{
		return mIndex;
	}

	void MirrorMaterial::setIndexData(SpectralShaderOutput* data)
	{
		mIndex = data;
	}

	Spectrum MirrorMaterial::apply(const SamplePoint& point, const PM::vec3& L)
	{
		if (mSpecularity)
			return mSpecularity->eval(point);
		else
			return Spectrum();
	}

	float MirrorMaterial::pdf(const SamplePoint& point, const PM::vec3& L)
	{
		return std::numeric_limits<float>::infinity();
	}

	PM::vec3 MirrorMaterial::sample(const SamplePoint& point, const PM::vec3& rnd, float& pdf)
	{
		auto dir = Reflection::reflect(point.NdotV, point.N, point.V);

		pdf = std::numeric_limits<float>::infinity();
		return dir;
	}
}