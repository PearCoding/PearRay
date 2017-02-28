#include "WardMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/WardMaterial.h"

#include "DataLisp.h"

using namespace PR;
namespace PRU
{
	std::shared_ptr<PR::Material> WardMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data albedoD = group.getFromKey("albedo");
		DL::Data specularityD = group.getFromKey("specularity");
		DL::Data roughnessD = group.getFromKey("roughness");
		DL::Data roughnessXD = group.getFromKey("roughness_x");
		DL::Data roughnessYD = group.getFromKey("roughness_x");
		DL::Data reflectivityD = group.getFromKey("reflectivity");

		auto diff = std::make_shared<WardMaterial>(env->materialCount() + 1);

		diff->setAlbedo(loader->getSpectralOutput(env, albedoD));
		diff->setSpecularity(loader->getSpectralOutput(env, specularityD));
		diff->setReflectivity(loader->getScalarOutput(env, reflectivityD));

		if (roughnessD.isValid() && !roughnessXD.isValid() && !roughnessYD.isValid())
		{
			auto roughness = loader->getScalarOutput(env, roughnessD);
			diff->setRoughnessX(roughness);
			diff->setRoughnessY(roughness);
		}
		else if (!roughnessD.isValid() && roughnessXD.isValid() && roughnessYD.isValid())
		{
			diff->setRoughnessX(loader->getScalarOutput(env, roughnessXD));
			diff->setRoughnessY(loader->getScalarOutput(env, roughnessYD));
		}
		else if (roughnessD.isValid() || roughnessXD.isValid() || roughnessYD.isValid())
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Ward: Isotropic and Anisotropic mismatch.");
		}

		return diff;
	}
}