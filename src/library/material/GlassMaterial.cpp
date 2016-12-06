#include "GlassMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSettings.h"
#include "entity/RenderEntity.h"

#include "math/Reflection.h"
#include "math/Fresnel.h"

#include "Logger.h"
#include "CTP.h"

namespace PR
{
	GlassMaterial::GlassMaterial(uint32 id) :
		Material(id), 
		mSpecularity(nullptr), mIndex(nullptr), mSampleIOR(false)
	{
	}
	
	bool GlassMaterial::sampleIOR() const
	{
		return mSampleIOR;
	}

	void GlassMaterial::setSampleIOR(bool b)
	{
		mSampleIOR = b;
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
		float weight = (point.Flags & SCF_Inside) == 0 ?
			Fresnel::dielectric(point.NdotV, 1, ind) : Fresnel::dielectric(point.NdotV, ind, 1);
		
		PM::vec3 dir;
		if (PM::pm_GetY(rnd) < weight)
			dir = Reflection::reflect(point.NdotV, point.N, point.V);
		else
			dir = Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1/ind : ind, point.NdotV, point.N, point.V);

		pdf = std::numeric_limits<float>::infinity();
		return dir;
	}

	PM::vec3 GlassMaterial::samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, Spectrum& path_weight, uint32 path)
	{
		uint32 lambda = 0;
		if(mSampleIOR)
			lambda = PM::pm_Min<uint32>(
				(path/2)*mIntervalLength_Cache + PM::pm_GetX(rnd)*mIntervalLength_Cache,
				Spectrum::SAMPLING_COUNT-1);
		else
			lambda = PM::pm_GetX(rnd)*Spectrum::SAMPLING_COUNT;

		const float ind = mIndex ? mIndex->eval(point).value(lambda) : 1.55f;
		float weight = (point.Flags & SCF_Inside) == 0 ?
			Fresnel::dielectric(point.NdotV, 1, ind) : Fresnel::dielectric(point.NdotV, ind, 1);
		
		PM::vec3 dir;
		if (path % 2 == 0)
			dir = Reflection::reflect(point.NdotV, point.N, point.V);
		else
		{
			weight = 1-weight;
			dir = Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1/ind : ind, point.NdotV, point.N, point.V);
		}

		if(mSampleIOR)
		{
			const uint32 index = (path/2)*mIntervalLength_Cache;
			path_weight = Spectrum::fromRectangularFunction(index, index+mIntervalLength_Cache) * weight;
			//weight /= mIntervalCount_Cache;
		}
		else
		{
			path_weight.fill(weight);
		}

		pdf = std::numeric_limits<float>::infinity();
		return dir;
	}

	uint32 GlassMaterial::samplePathCount() const
	{
		return (mIndex && mSampleIOR) ? 2*mIntervalCount_Cache : 2;
	}

	typedef CTP::Divisor<Spectrum::SAMPLING_COUNT> SCDivisor;
	void GlassMaterial::setup(RenderContext* context)
	{
		mIntervalCount_Cache = SCDivisor::values[PM::pm_Clamp<uint32>(
			std::floor(context->settings().distortionQuality() * SCDivisor::length),
			0, SCDivisor::length-1)];
		
		mIntervalLength_Cache = Spectrum::SAMPLING_COUNT / mIntervalCount_Cache;
	
		PR_LOGGER.logf(L_Info, M_Material,
			"Glass Interval Length %i -> Count %i",
				mIntervalLength_Cache, mIntervalCount_Cache);
	}
}