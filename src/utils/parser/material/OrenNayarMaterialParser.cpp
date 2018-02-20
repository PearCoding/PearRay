#include "OrenNayarMaterialParser.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "Logger.h"

#include "material/OrenNayarMaterial.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<Material> OrenNayarMaterialParser::parse(Environment* env,
														 const std::string& obj, const DL::DataGroup& group) const
{
	DL::Data albedoD	= group.getFromKey("albedo");
	DL::Data roughnessD = group.getFromKey("roughness");

	auto diff = std::make_shared<OrenNayarMaterial>(env->materialCount() + 1);

	diff->setAlbedo(SceneLoader::getSpectralOutput(env, albedoD));
	diff->setRoughness(SceneLoader::getScalarOutput(env, roughnessD));
	return diff;
}
} // namespace PR
