#include "OrenNayarMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/OrenNayarMaterial.h"

#include "DataLisp.h"

using namespace PR;
namespace PRU
{
	std::shared_ptr<PR::Material> OrenNayarMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data albedoD = group.getFromKey("albedo");
		DL::Data roughnessD = group.getFromKey("roughness");

		auto diff = std::make_shared<OrenNayarMaterial>(env->materialCount() + 1);

		diff->setAlbedo(loader->getSpectralOutput(env, albedoD));
		diff->setRoughness(loader->getScalarOutput(env, roughnessD));
		return diff;
	}
}