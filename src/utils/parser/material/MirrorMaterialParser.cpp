#include "MirrorMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/MirrorMaterial.h"

#include "DataLisp.h"

using namespace PR;
namespace PRU
{
	Material* MirrorMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data specD = group.getFromKey("specularity");
		DL::Data indexD = group.getFromKey("index");

		MirrorMaterial* diff = new MirrorMaterial(env->materialCount() + 1);

		diff->setSpecularity(loader->getSpectralOutput(env, specD));
		diff->setIndexData(loader->getSpectralOutput(env, indexD, true));

		return diff;
	}
}