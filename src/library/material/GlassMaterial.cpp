#include "GlassMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

#include "math/Reflection.h"
#include "math/Fresnel.h"

namespace PR
{
	GlassMaterial::GlassMaterial() :
		Material(), 
		mSpecularity(nullptr), mIndex(nullptr)
	{
	}

	SpectralShaderOutput* GlassMaterial::specularity() const
	{
		return mSpecularity;
	}

	void GlassMaterial::setSpecularity(SpectralShaderOutput* spec)
	{
		mSpecularity = spec;
	}

	SpectralShaderOutput* GlassMaterial::indexData() const
	{
		return mIndex;
	}

	void GlassMaterial::setIndexData(SpectralShaderOutput* data)
	{
		mIndex = data;
	}

	Spectrum GlassMaterial::eval(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		if (mSpecularity)
			return mSpecularity->eval(point);
		else
			return Spectrum();
	}

	float GlassMaterial::pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		return std::numeric_limits<float>::infinity();
	}

	PM::vec3 GlassMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		const float ind = mIndex ? mIndex->eval(point).value(PM::pm_GetX(rnd)*Spectrum::SAMPLING_COUNT) : 1.55f;
		const float d = (point.Flags & SCF_Inside) == 0 ?
			Fresnel::dielectric(point.NdotV, 1, ind) : Fresnel::dielectric(point.NdotV, ind, 1);

		PM::vec3 dir;
		if (PM::pm_GetY(rnd) < d)
			dir = Reflection::reflect(point.NdotV, point.N, point.V);
		else
			dir = Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1/ind : ind, point.NdotV, point.N, point.V);

		pdf = std::numeric_limits<float>::infinity();
		return dir;
	}

	PM::vec3 GlassMaterial::samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, float& weight, uint32 path)
	{
		const float ind = mIndex ? mIndex->eval(point).value(PM::pm_GetX(rnd)*Spectrum::SAMPLING_COUNT) : 1.55f;
		weight = (point.Flags & SCF_Inside) == 0 ?
			Fresnel::dielectric(point.NdotV, 1, ind) : Fresnel::dielectric(point.NdotV, ind, 1);
		
		PM::vec3 dir;
		if (path == 0)
			dir = Reflection::reflect(point.NdotV, point.N, point.V);
		else
		{
			weight = 1-weight;
			dir = Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1/ind : ind, point.NdotV, point.N, point.V);
		}

		pdf = std::numeric_limits<float>::infinity();
		return PM::pm_Normalize3D(dir);
	}

	uint32 GlassMaterial::samplePathCount() const
	{
		return 2;
	}
}