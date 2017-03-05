#include "EnvironmentLightParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "light/EnvironmentLight.h"

#include "DataLisp.h"

#include <algorithm>

namespace PR
{
	std::shared_ptr<PR::IInfiniteLight> EnvironmentLightParser::parse(Environment* env, const DL::DataGroup& group) const
	{
		DL::Data matD = group.getFromKey("material");

		auto light = std::make_shared<EnvironmentLight>();

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