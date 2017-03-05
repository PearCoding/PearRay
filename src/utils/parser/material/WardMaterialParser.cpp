#include "WardMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/WardMaterial.h"

#include "DataLisp.h"

namespace PR
{
	std::shared_ptr<PR::Material> WardMaterialParser::parse(Environment* env,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data albedoD = group.getFromKey("albedo");
		DL::Data specularityD = group.getFromKey("specularity");
		DL::Data roughnessD = group.getFromKey("roughness");
		DL::Data roughnessXD = group.getFromKey("roughness_x");
		DL::Data roughnessYD = group.getFromKey("roughness_x");
		DL::Data reflectivityD = group.getFromKey("reflectivity");

		auto diff = std::make_shared<WardMaterial>(env->materialCount() + 1);

		diff->setAlbedo(SceneLoader::getSpectralOutput(env, albedoD));
		diff->setSpecularity(SceneLoader::getSpectralOutput(env, specularityD));
		diff->setReflectivity(SceneLoader::getScalarOutput(env, reflectivityD));

		if (roughnessD.isValid() && !roughnessXD.isValid() && !roughnessYD.isValid())
		{
			auto roughness = SceneLoader::getScalarOutput(env, roughnessD);
			diff->setRoughnessX(roughness);
			diff->setRoughnessY(roughness);
		}
		else if (!roughnessD.isValid() && roughnessXD.isValid() && roughnessYD.isValid())
		{
			diff->setRoughnessX(SceneLoader::getScalarOutput(env, roughnessXD));
			diff->setRoughnessY(SceneLoader::getScalarOutput(env, roughnessYD));
		}
		else if (roughnessD.isValid() || roughnessXD.isValid() || roughnessYD.isValid())
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Ward: Isotropic and Anisotropic mismatch.");
		}

		return diff;
	}
}