#include "BlinnPhongMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/BlinnPhongMaterial.h"

#include "DataLisp.h"

using namespace PR;
namespace PRU
{
	Material* BlinnPhongMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data albedoD = group.getFromKey("albedo");
		DL::Data indexD = group.getFromKey("index");
		DL::Data shininessD = group.getFromKey("shininess");

		BlinnPhongMaterial* diff = new BlinnPhongMaterial(env->materialCount() + 1);

		diff->setAlbedo(loader->getSpectralOutput(env, albedoD));
		diff->setShininess(loader->getScalarOutput(env, shininessD));
		diff->setFresnelIndex(loader->getSpectralOutput(env, indexD, true));
		return diff;
	}
}