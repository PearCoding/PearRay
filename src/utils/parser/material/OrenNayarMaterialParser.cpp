#include "OrenNayarMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/OrenNayarMaterial.h"

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
	Material* OrenNayarMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{

		DL::Data* albedoD = group->getFromKey("albedo");
		DL::Data* roughnessD = group->getFromKey("roughness");

		OrenNayarMaterial* diff = new OrenNayarMaterial;

		diff->setAlbedo(loader->getSpectralOutput(env, albedoD));
		diff->setRoughness(loader->getScalarOutput(env, roughnessD));
		return diff;
	}
}