#include "DistantLightParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "light/DistantLight.h"

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
	PR::IInfiniteLight* DistantLightParser::parse(SceneLoader* loader, Environment* env, DL::DataGroup* group) const
	{
		DL::Data dirD = group->getFromKey("direction");
		DL::Data matD = group->getFromKey("material");

		DistantLight* light = new DistantLight();

		if (matD.type() == DL::Data::T_String)
		{
			if (env->hasMaterial(matD.getString()))
				light->setMaterial(env->getMaterial(matD.getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", matD.getString().c_str());
		}

		if (dirD.type() == DL::Data::T_Array)
		{
			bool ok;
			auto v = loader->getVector(dirD.getArray(), ok);
			if(ok)
				light->setDirection(v);
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Invalid direction given.");
		}

		return light;
	}
}