#include "BRDFMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/BRDFMaterial.h"

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
	Material* BRDFMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{

		DL::Data* albedoD = group->getFromKey("albedo");
		DL::Data* specD = group->getFromKey("specularity");
		DL::Data* roughnessD = group->getFromKey("roughness");
		DL::Data* reflectivityD = group->getFromKey("reflectivity");
		DL::Data* fresnelD = group->getFromKey("fresnel");

		BRDFMaterial* diff = new BRDFMaterial;

		diff->setAlbedo(loader->getTexture2D(env, albedoD));		
		diff->setSpecularity(loader->getTexture2D(env, specD));
		
		diff->setRoughnessData(loader->getData2D(env, roughnessD));
		diff->setReflectivityData(loader->getData2D(env, reflectivityD));
		diff->setFresnelData(loader->getData1D(env, fresnelD));

		return diff;
	}
}