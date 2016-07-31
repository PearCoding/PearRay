#include "WardMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/WardMaterial.h"

// DataLisp
#include "DataLisp.h"
#include "DataContainer.h"
#include "DataGroup.h"
#include "DataArray.h"
#include "Data.h"
#include "SourceLogger.h"

using namespace PR;
namespace PRU
{
	Material* WardMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{

		DL::Data* albedoD = group->getFromKey("albedo");
		DL::Data* specularityD = group->getFromKey("specularity");
		DL::Data* roughnessD = group->getFromKey("roughness");
		DL::Data* roughnessXD = group->getFromKey("roughnessX");
		DL::Data* roughnessYD = group->getFromKey("roughnessY");

		WardMaterial* diff = new WardMaterial;

		diff->setAlbedo(loader->getSpectralOutput(env, albedoD));
		diff->setSpecularity(loader->getSpectralOutput(env, specularityD));

		if (roughnessD && !roughnessXD && !roughnessYD)
		{
			auto roughness = loader->getScalarOutput(env, roughnessD);
			diff->setRoughnessX(roughness);
			diff->setRoughnessY(roughness);
		}
		else if (!roughnessD && roughnessXD && roughnessYD)
		{
			diff->setRoughnessX(loader->getScalarOutput(env, roughnessXD));
			diff->setRoughnessY(loader->getScalarOutput(env, roughnessYD));
		}
		else if (roughnessD || roughnessXD || roughnessYD)
		{
			PR_LOGGER.logf(L_Warning, M_Scene, "Ward: Isotropic and Anisotropic mismatch.");
		}

		return diff;
	}
}