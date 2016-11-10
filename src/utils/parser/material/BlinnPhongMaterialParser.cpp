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
		DL::Data albedoD = group->getFromKey("albedo");
		DL::Data indexD = group->getFromKey("index");
		DL::Data shininessD = group->getFromKey("shininess");

		BlinnPhongMaterial* diff = new BlinnPhongMaterial(env->materialCount() + 1);

		diff->setAlbedo(loader->getSpectralOutput(env, albedoD));
		diff->setShininess(loader->getScalarOutput(env, shininessD));
		diff->setFresnelIndex(loader->getSpectralOutput(env, indexD, true));
		return diff;
	}
}