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
		DL::Data* matD = group->getFromKey("material");
		DL::Data* backgroundD = group->getFromKey("background");

		EnvironmentLight* light = new EnvironmentLight();

		if (matD && matD->isType() == DL::Data::T_String)
		{
			if (env->hasMaterial(matD->getString()))
				light->setMaterial(env->getMaterial(matD->getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", matD->getString().c_str());
		}

		if (backgroundD && backgroundD->isType() == DL::Data::T_Bool && backgroundD->getBool())
		{
			if(env->scene()->backgroundLight())
				PR_LOGGER.logf(L_Warning, M_Scene, "A background is already assigned! Overriding it.");
			
			env->scene()->setBackgroundLight(light);
		}

		return light;
	}
}