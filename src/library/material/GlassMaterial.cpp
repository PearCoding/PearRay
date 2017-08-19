#include "GlassMaterial.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSettings.h"
#include "shader/ShaderClosure.h"

#include "math/Fresnel.h"
#include "math/Reflection.h"

#include "CTP.h"
#include "Logger.h"

#include <sstream>

#define PR_GLASS_USE_DEFAULT_SCHLICK

namespace PR {
GlassMaterial::GlassMaterial(uint32 id)
	: Material(id)
	, mSpecularity(nullptr)
	, mIndex(nullptr)
	, mThin(false)
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

const std::shared_ptr<SpectralShaderOutput>& GlassMaterial::specularity() const
{
	return mSpecularity;
}

void GlassMaterial::setSpecularity(const std::shared_ptr<SpectralShaderOutput>& spec)
{
	mSpecularity = spec;
}

const std::shared_ptr<SpectralShaderOutput>& GlassMaterial::ior() const
{
	return mIndex;
}

void GlassMaterial::setIOR(const std::shared_ptr<SpectralShaderOutput>& data)
{
	mIndex = data;
}

Spectrum GlassMaterial::eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
{
	if (mSpecularity)
		return mSpecularity->eval(point);
	else
		return Spectrum();
}

float GlassMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
{
	return std::numeric_limits<float>::infinity();
}

MaterialSample GlassMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd)
{
	const float ind = mIndex ? mIndex->eval(point).value(point.WavelengthIndex) : 1.55f;
	float weight	= (point.Flags & SCF_Inside) == 0 ?
#ifndef PR_GLASS_USE_DEFAULT_SCHLICK
												   Fresnel::dielectric(-point.NdotV, 1, ind)
												   : Fresnel::dielectric(-point.NdotV, ind, 1);
#else
												   Fresnel::schlick(-point.NdotV, 1, ind)
												   : Fresnel::schlick(-point.NdotV, ind, 1);
#endif

	bool total = false;
	MaterialSample ms;
	if (rnd(1) < weight) {
		ms.L = Reflection::reflect(point.NdotV, point.N, point.V);
	} else {
		ms.L = Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1 / ind : ind,
								  point.NdotV, point.N, point.V, total);
		if (!mThin && total)
			ms.L = Reflection::reflect(point.NdotV, point.N, point.V);
	}

	ms.PDF_S = (total && mThin) ? 0 : std::numeric_limits<float>::infinity();
	return ms;
}

MaterialSample GlassMaterial::samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path)
{
	MaterialSample ms;
	const float ind = mIndex ? mIndex->eval(point).value(point.WavelengthIndex) : 1.55f;
	ms.Weight		= (point.Flags & SCF_Inside) == 0 ?
#ifndef PR_GLASS_USE_DEFAULT_SCHLICK
												  Fresnel::dielectric(-point.NdotV, 1, ind)
												  : Fresnel::dielectric(-point.NdotV, ind, 1);
#else
												  Fresnel::schlick(-point.NdotV, 1, ind)
												  : Fresnel::schlick(-point.NdotV, ind, 1);
#endif

	bool total = false;
	if (path == 0) {
		ms.L = Reflection::reflect(point.NdotV, point.N, point.V);
	} else {
		ms.Weight = 1 - ms.Weight;
		ms.L			= Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1 / ind : ind,
								  point.NdotV, point.N, point.V, total);
		if (!mThin && total)
			ms.L = Reflection::reflect(point.NdotV, point.N, point.V);
	}

	ms.PDF_S = (total && mThin) ? 0 : std::numeric_limits<float>::infinity();
	return ms;
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
