#include "DebugBoundingBoxMaterialParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "material/DebugBoundingBoxMaterial.h"

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
	Material* DebugBoundingBoxMaterialParser::parse(SceneLoader* loader, Environment* env,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data* colorD = group->getFromKey("color");
		DL::Data* densityD = group->getFromKey("density");

		DebugBoundingBoxMaterial* dbbm = new DebugBoundingBoxMaterial();

		if (colorD && colorD->isType() == DL::Data::T_String)
		{
			if (env->hasSpectrum(colorD->getString()))
			{
				dbbm->setColor(env->getSpectrum(colorD->getString()));
			}
			else
			{
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find spectrum '%s' for material",
					colorD->getString().c_str());
			}
		}

		if (densityD && densityD->isNumber())
		{
			dbbm->setDensity(densityD->getFloatConverted());
		}

		return dbbm;
	}
}