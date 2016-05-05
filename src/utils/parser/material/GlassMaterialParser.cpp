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
		DL::Data* specD = group->getFromKey("specularity");
		DL::Data* indexD = group->getFromKey("index");

		GlassMaterial* diff = new GlassMaterial;

		if (specD && specD->isType() == DL::Data::T_String)
		{
			if (env->hasSpectrum(specD->getString()))
			{
				diff->setSpecularity(env->getSpectrum(specD->getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
					specD->getString().c_str());
			}
		}

		if (indexD && indexD->isNumber())
		{
			diff->setIndex(indexD->getFloatConverted());
		}

		return diff;
	}
}