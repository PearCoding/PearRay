#include "GlassMaterial.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "renderer/RenderSettings.h"

#include "shader/ConstSpectralOutput.h"
#include "shader/ShaderClosure.h"

#include "math/Fresnel.h"
#include "math/Reflection.h"

#include "Logger.h"

#include <sstream>

#define PR_GLASS_USE_DEFAULT_SCHLICK

namespace PR {
GlassMaterial::GlassMaterial(uint32 id)
	: Material(id)
	, mSpecularity(nullptr)
	, mIndex(nullptr)
	, mThin(false)
	, mSpectrumSamples(0)
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

std::shared_ptr<SpectrumShaderOutput> GlassMaterial::specularity() const
{
	return mSpecularity;
}

void GlassMaterial::setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec)
{
	mSpecularity = spec;
}

std::shared_ptr<SpectrumShaderOutput> GlassMaterial::ior() const
{
	return mIndex;
}

void GlassMaterial::setIOR(const std::shared_ptr<SpectrumShaderOutput>& data)
{
	mIndex = data;
}

void GlassMaterial::onFreeze(RenderContext* context)
{
	mSpectrumSamples = context->spectrumDescriptor()->samples();

	if (!mSpecularity)
		mSpecularity = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::white(context->spectrumDescriptor()));

	if (!mIndex)
		mIndex = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::gray(context->spectrumDescriptor(), 1.55f));
}

void GlassMaterial::eval(
	Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L,
	float NdotL, const RenderSession& session) const
{
	mSpecularity->eval(spec, point);
}

float GlassMaterial::pdf(
	const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const
{
	return std::numeric_limits<float>::infinity();
}

MaterialSample GlassMaterial::sample(
	const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) const
{
	const float ind   = mIndex->evalIndex(point, point.WavelengthIndex, mSpectrumSamples);
	const float eta   = (point.Flags & SCF_Inside) == 0 ? 1 / ind : ind;
	const float NdotT = Reflection::refraction_angle(point.NdotV, eta);

	MaterialSample ms;
	if (NdotT < 0) { // Total reflection
		if (!mThin) {
			ms.PathWeight = 1;
			sample_reflection(ms, point, session);
		} else {
			ms.PathWeight = 0;
		}
	} else {
		ms.PathWeight = get_weight(ind, NdotT, point);

		if (rnd(0) <= ms.PathWeight) {
			sample_reflection(ms, point, session);
		} else {
			ms.PathWeight = 1 - ms.PathWeight;
			sample_refraction(ms, NdotT, eta, point, session);
		}
	}

	PR_ASSERT(ms.PathWeight >= 0 && ms.PathWeight <= 1, "Weight should be bounded to [0,1]");
	ms.PDF_S = std::numeric_limits<float>::infinity();

	return ms;
}

MaterialSample GlassMaterial::samplePath(
	const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path, const RenderSession& session) const
{
	const float ind   = mIndex->evalIndex(point, point.WavelengthIndex, mSpectrumSamples);
	const float eta   = (!point.isInside()) ? 1 / ind : ind;
	const float NdotT = Reflection::refraction_angle(point.NdotV, eta);

	MaterialSample ms;
	if (NdotT < 0) { // Total reflection
		if (!mThin && path == 0) {
			ms.PathWeight = 1;
			sample_reflection(ms, point, session);
		} else {
			ms.PathWeight = 0;
		}
	} else {
		ms.PathWeight = get_weight(ind, NdotT, point);

		if (path == 0) {
			sample_reflection(ms, point, session);
		} else {
			ms.PathWeight = 1.0f - ms.PathWeight;
			sample_refraction(ms, NdotT, eta, point, session);
		}
	}

	PR_ASSERT(ms.PathWeight >= 0 && ms.PathWeight <= 1, "Weight should be bounded to [0,1]");

	ms.PDF_S = std::numeric_limits<float>::infinity();
	return ms;
}

void GlassMaterial::sample_reflection(
	MaterialSample& ms, const ShaderClosure& point, const RenderSession& session) const
{
	ms.ScatteringType = MST_SpecularReflection;
	ms.L			  = Reflection::reflect(point.NdotV, point.N, point.V);
}

void GlassMaterial::sample_refraction(
	MaterialSample& ms, float NdotT, float eta, const ShaderClosure& point, const RenderSession& session) const
{
	ms.ScatteringType = MST_SpecularTransmission;
	ms.L			  = Reflection::refract(eta, point.NdotV, NdotT, point.N, point.V);
}

float GlassMaterial::get_weight(float ior, float NdotT, const ShaderClosure& point) const
{
	return (point.Flags & SCF_Inside) == 0 ?
#ifndef PR_GLASS_USE_DEFAULT_SCHLICK
										   Fresnel::dielectric(-point.NdotV, NdotT, 1, ior)
										   : Fresnel::dielectric(-point.NdotV, NdotT, ior, 1);
#else
										   Fresnel::schlick(-point.NdotV, 1, ior)
										   : Fresnel::schlick(-point.NdotV, ior, 1);
#endif
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
