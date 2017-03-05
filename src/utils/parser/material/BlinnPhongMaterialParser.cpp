#include "BlinnPhongMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/BlinnPhongMaterial.h"

#include "DataLisp.h"

namespace PR
{
	std::shared_ptr<PR::Material> BlinnPhongMaterialParser::parse(Environment* env,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data albedoD = group.getFromKey("albedo");
		DL::Data indexD = group.getFromKey("index");
		DL::Data shininessD = group.getFromKey("shininess");

		auto diff = std::make_shared<BlinnPhongMaterial>(env->materialCount() + 1);

		diff->setAlbedo(SceneLoader::getSpectralOutput(env, albedoD));
		diff->setShininess(SceneLoader::getScalarOutput(env, shininessD));
		diff->setFresnelIndex(SceneLoader::getSpectralOutput(env, indexD, true));
		return diff;
	}
}