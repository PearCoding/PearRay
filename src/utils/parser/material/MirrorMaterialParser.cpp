#include "MirrorMaterialParser.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "Logger.h"

#include "material/MirrorMaterial.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<PR::Material> MirrorMaterialParser::parse(Environment* env,
														  const std::string& obj, const DL::DataGroup& group) const
{
	DL::Data specD  = group.getFromKey("specularity");
	DL::Data indexD = group.getFromKey("index");

	auto diff = std::make_shared<MirrorMaterial>(env->materialCount() + 1);

	diff->setSpecularity(SceneLoader::getSpectralOutput(env, specD));
	diff->setIOR(SceneLoader::getSpectralOutput(env, indexD, true));

	return diff;
}
} // namespace PR
