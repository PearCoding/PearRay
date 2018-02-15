#include "DiffuseMaterialParser.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "Logger.h"

#include "material/DiffuseMaterial.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<PR::Material> DiffuseMaterialParser::parse(Environment* env,
														   const std::string& obj, const DL::DataGroup& group) const
{
	DL::Data albedoD = group.getFromKey("albedo");

	auto diff = std::make_shared<DiffuseMaterial>(env->materialCount() + 1);

	diff->setAlbedo(SceneLoader::getSpectralOutput(env, albedoD));
	return diff;
}
} // namespace PR
