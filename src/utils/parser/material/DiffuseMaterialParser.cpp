#include "DiffuseMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/DiffuseMaterial.h"

#include "DataLisp.h"

using namespace PR;
namespace PRU
{
	std::shared_ptr<PR::Material> DiffuseMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data albedoD = group.getFromKey("albedo");

		auto diff = std::make_shared<DiffuseMaterial>(env->materialCount() + 1);

		diff->setAlbedo(loader->getSpectralOutput(env, albedoD));
		return diff;
	}
}