#include "MirrorMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

#include "BRDF.h"
#include "math/Reflection.h"
#include "math/Fresnel.h"

namespace PR
{
	MirrorMaterial::MirrorMaterial(uint32 id) :
		Material(id), 
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

	Spectrum MirrorMaterial::eval(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		if (mSpecularity)
			return mSpecularity->eval(point);
		else
			return Spectrum();
	}

	float MirrorMaterial::pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		return std::numeric_limits<float>::infinity();
	}

	PM::vec3 MirrorMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		pdf = std::numeric_limits<float>::infinity();
		return Reflection::reflect(point.NdotV, point.N, point.V);
	}
}