#include "GlassMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/GlassMaterial.h"

#include "DataLisp.h"

using namespace PR;
namespace PRU
{
	std::shared_ptr<PR::Material> GlassMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data specD = group.getFromKey("specularity");
		DL::Data indexD = group.getFromKey("index");
		DL::Data thinD = group.getFromKey("thin");

		auto diff = std::make_shared<GlassMaterial>(env->materialCount() + 1);

		diff->setSpecularity(loader->getSpectralOutput(env, specD));
		diff->setIndexData(loader->getSpectralOutput(env, indexD, true));

		if(thinD.type() == DL::Data::T_Bool)
			diff->setThin(thinD.getBool());

		return diff;
	}
}