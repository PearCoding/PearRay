#include "MirrorMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/MirrorMaterial.h"

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
	Material* MirrorMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data* specD = group->getFromKey("specularity");
		DL::Data* indexD = group->getFromKey("index");

		MirrorMaterial* diff = new MirrorMaterial;

		diff->setSpecularity(loader->getTexture2D(env, specD));
		diff->setIndexData(loader->getData1D(env, indexD));

		return diff;
	}
}