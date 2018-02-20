#include "MirrorMaterial.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "shader/ConstSpectralOutput.h"
#include "shader/ShaderClosure.h"

#include "BRDF.h"
#include "math/Fresnel.h"
#include "math/Reflection.h"

#include <sstream>

namespace PR {
MirrorMaterial::MirrorMaterial(uint32 id)
	: Material(id)
	, mSpecularity(nullptr)
	, mIndex(nullptr)
{
}

std::shared_ptr<SpectrumShaderOutput> MirrorMaterial::specularity() const
{
	return mSpecularity;
}

void MirrorMaterial::setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec)
{
	mSpecularity = spec;
}

std::shared_ptr<SpectrumShaderOutput> MirrorMaterial::ior() const
{
	return mIndex;
}

void MirrorMaterial::setIOR(const std::shared_ptr<SpectrumShaderOutput>& data)
{
	mIndex = data;
}

void MirrorMaterial::setup(RenderContext* context)
{
	if (!mSpecularity)
		mSpecularity = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::white(context->spectrumDescriptor()));
}

void MirrorMaterial::eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	mSpecularity->eval(spec, point);
}

float MirrorMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	return std::numeric_limits<float>::infinity();
}

MaterialSample MirrorMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session)
{
	MaterialSample ms;
	ms.PDF_S		  = std::numeric_limits<float>::infinity();
	ms.ScatteringType = MST_SpecularReflection;
	ms.L			  = Reflection::reflect(point.NdotV, point.N, point.V);
	return ms;
}

std::string MirrorMaterial::dumpInformation() const
{
	std::stringstream stream;

	stream << std::boolalpha << Material::dumpInformation()
		   << "  <MirrorMaterial>:" << std::endl
		   << "    HasSpecularity:   " << (mSpecularity ? "true" : "false") << std::endl
		   << "    HasIOR:           " << (mIndex ? "true" : "false") << std::endl;
	//<< "    SampleIOR:        " << sampleIOR() << std::endl
	//<< "    [IntervalCount]:  " << mIntervalCount_Cache << std::endl
	//<< "    [IntervalLength]: " << mIntervalLength_Cache << std::endl;

	return stream.str();
}
} // namespace PR
