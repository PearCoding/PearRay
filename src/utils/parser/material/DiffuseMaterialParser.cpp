#include "DiffuseMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/DiffuseMaterial.h"

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
	Material* DiffuseMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{

		DL::Data* albedoD = group->getFromKey("albedo");

		DiffuseMaterial* diff = new DiffuseMaterial;

		diff->setAlbedo(loader->getTexture2D(env, albedoD));
		return diff;
	}
}