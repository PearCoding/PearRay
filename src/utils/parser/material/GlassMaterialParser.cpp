#include "GlassMaterialParser.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "Logger.h"

#include "material/GlassMaterial.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<PR::Material> GlassMaterialParser::parse(Environment* env,
														 const std::string& obj, const DL::DataGroup& group) const
{
	DL::Data specD  = group.getFromKey("specularity");
	DL::Data indexD = group.getFromKey("index");
	DL::Data thinD  = group.getFromKey("thin");

	auto diff = std::make_shared<GlassMaterial>(env->materialCount() + 1);

	diff->setSpecularity(SceneLoader::getSpectralOutput(env, specD));
	diff->setIOR(SceneLoader::getSpectralOutput(env, indexD, true));

	if (thinD.type() == DL::Data::T_Bool)
		diff->setThin(thinD.getBool());

	return diff;
}
} // namespace PR
