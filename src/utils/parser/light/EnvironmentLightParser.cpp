#include "EnvironmentLightParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "light/EnvironmentLight.h"

// DataLisp
#include "DataLisp.h"
#include "DataContainer.h"
#include "DataGroup.h"
#include "DataArray.h"
#include "Data.h"
#include "SourceLogger.h"

#include <algorithm>

using namespace PR;
namespace PRU
{
	PR::IInfiniteLight* EnvironmentLightParser::parse(SceneLoader* loader, Environment* env, DL::DataGroup* group) const
	{
		DL::Data matD = group->getFromKey("material");

		EnvironmentLight* light = new EnvironmentLight();

		if (matD.type() == DL::Data::T_String)
		{
			if (env->hasMaterial(matD.getString()))
				light->setMaterial(env->getMaterial(matD.getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", matD.getString().c_str());
		}

		return light;
	}
}