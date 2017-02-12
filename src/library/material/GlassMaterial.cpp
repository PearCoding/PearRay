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

#include <sstream>

#define PR_GLASS_USE_DEFAULT_SCHLICK

namespace PR
{
	GlassMaterial::GlassMaterial(uint32 id) :
		Material(id),
		mSpecularity(nullptr), mIndex(nullptr), mSampleIOR(false), mThin(false),
		mIntervalCount_Cache(0), mIntervalLength_Cache(0), mPathCount_Cache(0)
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

	bool GlassMaterial::isThin() const
	{
		return mThin;
	}

	void GlassMaterial::setThin(bool b)
	{
		mThin = b;
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
#ifndef PR_GLASS_USE_DEFAULT_SCHLICK
			Fresnel::dielectric(-point.NdotV, 1, ind) : Fresnel::dielectric(-point.NdotV, ind, 1);
#else
			Fresnel::schlick(-point.NdotV, 1, ind) : Fresnel::schlick(-point.NdotV, ind, 1);
#endif

		bool total = false;
		PM::vec3 dir;
		if (PM::pm_GetY(rnd) < weight)
			dir = Reflection::reflect(point.NdotV, point.N, point.V);
		else
		{
			dir = Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1/ind : ind,
				point.NdotV, point.N, point.V, total);
			if(!mThin && total)
				dir = Reflection::reflect(point.NdotV, point.N, point.V);
		}

		pdf = (total && mThin) ? 0 : std::numeric_limits<float>::infinity();
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
#ifndef PR_GLASS_USE_DEFAULT_SCHLICK
			Fresnel::dielectric(-point.NdotV, 1, ind) : Fresnel::dielectric(-point.NdotV, ind, 1);
#else
			Fresnel::schlick(-point.NdotV, 1, ind) : Fresnel::schlick(-point.NdotV, ind, 1);
#endif

		bool total = false;
		PM::vec3 dir;
		if (path % 2 == 0)
			dir = Reflection::reflect(point.NdotV, point.N, point.V);
		else
		{
			weight = 1-weight;
			dir = Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1/ind : ind,
				point.NdotV, point.N, point.V, total);
			if(!mThin && total)
				dir = Reflection::reflect(point.NdotV, point.N, point.V);
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

		pdf = (total && mThin) ? 0 : std::numeric_limits<float>::infinity();
		return dir;
	}

	uint32 GlassMaterial::samplePathCount() const
	{
		return mPathCount_Cache;
	}

	typedef CTP::Divisor<Spectrum::SAMPLING_COUNT> SCDivisor;
	void GlassMaterial::setup(RenderContext* context)
	{
		mIntervalCount_Cache = SCDivisor::values[PM::pm_Clamp<uint32>(
			std::floor(context->settings().distortionQuality() * SCDivisor::length),
			0, SCDivisor::length-1)];

		mIntervalLength_Cache = Spectrum::SAMPLING_COUNT / mIntervalCount_Cache;
		mPathCount_Cache = (mIndex && mSampleIOR) ? 2*mIntervalCount_Cache : 2;
	}

	std::string GlassMaterial::dumpInformation() const
	{
		std::stringstream stream;

		stream << std::boolalpha << Material::dumpInformation()
		    << "  <GlassMaterial>:" << std::endl
			<< "    HasSpecularity:   " << (mSpecularity ? "true" : "false") << std::endl
			<< "    HasIOR:           " << (mIndex ? "true" : "false") << std::endl
			<< "    SampleIOR:        " << sampleIOR() << std::endl
			<< "    IsThin:           " << isThin() << std::endl
			<< "    [IntervalCount]:  " << mIntervalCount_Cache << std::endl
			<< "    [IntervalLength]: " << mIntervalLength_Cache << std::endl;

		return stream.str();
	}
}
