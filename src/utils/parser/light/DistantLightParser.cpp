#include "DistantLightParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "light/DistantLight.h"

#include "DataLisp.h"

#include <algorithm>

using namespace PR;
namespace PRU
{
	std::shared_ptr<PR::IInfiniteLight> DistantLightParser::parse(SceneLoader* loader, Environment* env, const DL::DataGroup& group) const
	{
		DL::Data dirD = group.getFromKey("direction");
		DL::Data matD = group.getFromKey("material");

		auto light = std::make_shared<DistantLight>();

		if (matD.type() == DL::Data::T_String)
		{
			if (env->hasMaterial(matD.getString()))
				light->setMaterial(env->getMaterial(matD.getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", matD.getString().c_str());
		}

		if (dirD.type() == DL::Data::T_Group)
		{
			bool ok;
			auto v = loader->getVector(dirD.getGroup(), ok);
			if(ok)
				light->setDirection(v);
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Invalid direction given.");
		}

		return light;
	}
}