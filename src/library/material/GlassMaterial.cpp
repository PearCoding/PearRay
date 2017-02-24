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
		mSpecularity(nullptr), mIndex(nullptr), mThin(false)
	{
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
		const float ind = mIndex ? mIndex->eval(point).value(point.WavelengthIndex) : 1.55f;
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

	PM::vec3 GlassMaterial::samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, float& path_weight, uint32 path)
	{
		const float ind = mIndex ? mIndex->eval(point).value(point.WavelengthIndex) : 1.55f;
		path_weight = (point.Flags & SCF_Inside) == 0 ?
#ifndef PR_GLASS_USE_DEFAULT_SCHLICK
			Fresnel::dielectric(-point.NdotV, 1, ind) : Fresnel::dielectric(-point.NdotV, ind, 1);
#else
			Fresnel::schlick(-point.NdotV, 1, ind) : Fresnel::schlick(-point.NdotV, ind, 1);
#endif

		bool total = false;
		PM::vec3 dir;
		if (path == 0)
			dir = Reflection::reflect(point.NdotV, point.N, point.V);
		else
		{
			path_weight = 1-path_weight;
			dir = Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1/ind : ind,
				point.NdotV, point.N, point.V, total);
			if(!mThin && total)
				dir = Reflection::reflect(point.NdotV, point.N, point.V);
		}

		pdf = (total && mThin) ? 0 : std::numeric_limits<float>::infinity();
		return dir;
	}

	uint32 GlassMaterial::samplePathCount() const
	{
		return 2;
	}

	std::string GlassMaterial::dumpInformation() const
	{
		std::stringstream stream;

		stream << Material::dumpInformation()
		    << "  <GlassMaterial>:" << std::endl
			<< "    HasSpecularity:   " << (mSpecularity ? "true" : "false") << std::endl
			<< "    HasIOR:           " << (mIndex ? "true" : "false") << std::endl
			<< "    IsThin:           " << isThin() << std::endl;

		return stream.str();
	}
}
