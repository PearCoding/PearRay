#include "BlinnPhongMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/BlinnPhongMaterial.h"

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
	Material* BlinnPhongMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{

		DL::Data* albedoD = group->getFromKey("albedo");
		DL::Data* indexD = group->getFromKey("index");
		DL::Data* shininessD = group->getFromKey("shininess");

		BlinnPhongMaterial* diff = new BlinnPhongMaterial;

		diff->setAlbedo(loader->getTexture2D(env, albedoD));
		diff->setShininess(loader->getData2D(env, shininessD));
		diff->setFresnelIndex(loader->getData1D(env, indexD));
		return diff;
	}
}