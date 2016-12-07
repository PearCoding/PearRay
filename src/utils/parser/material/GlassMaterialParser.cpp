#include "GlassMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/GlassMaterial.h"

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
	Material* GlassMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data specD = group->getFromKey("specularity");
		DL::Data indexD = group->getFromKey("index");
		DL::Data sampleIORD = group->getFromKey("sample_index");
		DL::Data thinD = group->getFromKey("thin");

		GlassMaterial* diff = new GlassMaterial(env->materialCount() + 1);

		diff->setSpecularity(loader->getSpectralOutput(env, specD));
		diff->setIndexData(loader->getSpectralOutput(env, indexD, true));

		if(sampleIORD.type() == DL::Data::T_Bool)
			diff->setSampleIOR(sampleIORD.getBool());

		if(thinD.type() == DL::Data::T_Bool)
			diff->setThin(thinD.getBool());

		return diff;
	}
}