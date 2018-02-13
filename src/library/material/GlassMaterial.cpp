#include "GlassMaterial.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSettings.h"
#include "renderer/RenderSession.h"

#include "shader/ShaderClosure.h"
#include "shader/ConstSpectralOutput.h"

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

const std::shared_ptr<SpectrumShaderOutput>& GlassMaterial::specularity() const
{
	return mSpecularity;
}

void GlassMaterial::setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec)
{
	mSpecularity = spec;
}

const std::shared_ptr<SpectrumShaderOutput>& GlassMaterial::ior() const
{
	return mIndex;
}

void GlassMaterial::setIOR(const std::shared_ptr<SpectrumShaderOutput>& data)
{
	mIndex = data;
}

struct GM_ThreadData {
	Spectrum IOR;
	Spectrum Specularity;

	GM_ThreadData(RenderContext* context) :
		IOR(context->spectrumDescriptor()), Specularity(context->spectrumDescriptor()) {
	}
};

constexpr float MinRoughness = 0.001f;
void GlassMaterial::setup(RenderContext* context)
{
	mThreadData.clear();
	for(size_t i = 0; context->threads(); ++i) {
		mThreadData.emplace_back(context);
	}

	if(!mSpecularity)
		mSpecularity = std::make_shared<ConstSpectrumShaderOutput>(context->spectrumDescriptor()->fromWhite());

	if(!mIndex)
		mIndex = std::make_shared<ConstSpectrumShaderOutput>(context->spectrumDescriptor()->fromWhite()*1.55f);
}

void GlassMaterial::eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	mSpecularity->eval(spec, point);
}

float GlassMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	return std::numeric_limits<float>::infinity();
}

MaterialSample GlassMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session)
{
	GM_ThreadData& data = mThreadData[session.thread()];
	const float ind = data.IOR.value(point.WavelengthIndex);

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
	if (rnd(0) < weight) {
		ms.ScatteringType = MST_SpecularReflection;
		ms.L			  = Reflection::reflect(point.NdotV, point.N, point.V);
	} else {
		ms.ScatteringType = MST_SpecularTransmission;
		ms.L			  = Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1 / ind : ind,
								   -point.NdotV, point.N, point.V, total);
		if (!mThin && total)
			ms.L = Reflection::reflect(point.NdotV, point.N, point.V);
	}

	PR_ASSERT(weight >= 0 && weight <= 1, "Weight should be bounded to [0,1]");

	ms.PDF_S = (total && mThin) ? 0 : weight;
	return ms;
}

MaterialSample GlassMaterial::samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path, const RenderSession& session)
{
	GM_ThreadData& data = mThreadData[session.thread()];
	const float ind = data.IOR.value(point.WavelengthIndex);

	MaterialSample ms;
	float weight	= (point.Flags & SCF_Inside) == 0 ?
#ifndef PR_GLASS_USE_DEFAULT_SCHLICK
												   Fresnel::dielectric(-point.NdotV, 1, ind)
												   : Fresnel::dielectric(-point.NdotV, ind, 1);
#else
												   Fresnel::schlick(-point.NdotV, 1, ind)
												   : Fresnel::schlick(-point.NdotV, ind, 1);
#endif

	bool total = false;
	if (path == 0) {
		ms.ScatteringType = MST_SpecularReflection;
		ms.L			  = Reflection::reflect(point.NdotV, point.N, point.V);
	} else {
		weight			  = 1 - weight;
		ms.ScatteringType = MST_SpecularTransmission;
		ms.L			  = Reflection::refract((point.Flags & SCF_Inside) == 0 ? 1 / ind : ind,
								   -point.NdotV, point.N, point.V, total);
		if (!mThin && total)
			ms.L = Reflection::reflect(point.NdotV, point.N, point.V);
	}

	PR_ASSERT(weight >= 0 && weight <= 1, "Weight should be bounded to [0,1]");

	ms.PDF_S = (total && mThin) ? 0 : weight;
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
