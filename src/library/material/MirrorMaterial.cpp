#include "MirrorMaterial.h"
#include "entity/RenderEntity.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
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

const std::shared_ptr<SpectralShaderOutput>& MirrorMaterial::specularity() const
{
	return mSpecularity;
}

void MirrorMaterial::setSpecularity(const std::shared_ptr<SpectralShaderOutput>& spec)
{
	mSpecularity = spec;
}

const std::shared_ptr<SpectralShaderOutput>& MirrorMaterial::ior() const
{
	return mIndex;
}

void MirrorMaterial::setIOR(const std::shared_ptr<SpectralShaderOutput>& data)
{
	mIndex = data;
}

Spectrum MirrorMaterial::eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
{
	if (mSpecularity)
		return mSpecularity->eval(point);
	else
		return Spectrum();
}

float MirrorMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
{
	return std::numeric_limits<float>::infinity();
}

MaterialSample MirrorMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd)
{
	MaterialSample ms;
	ms.PDF_S = std::numeric_limits<float>::infinity();
	ms.L = Reflection::reflect(point.NdotV, point.N, point.V);
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
}
