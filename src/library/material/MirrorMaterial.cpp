#include "MirrorMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"
#include "renderer/RenderContext.h"
#include "entity/RenderEntity.h"

#include "BRDF.h"
#include "math/Reflection.h"
#include "math/Fresnel.h"

#include <sstream>

namespace PR
{
	MirrorMaterial::MirrorMaterial(uint32 id) :
		Material(id),
		mSpecularity(nullptr), mIndex(nullptr)
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
