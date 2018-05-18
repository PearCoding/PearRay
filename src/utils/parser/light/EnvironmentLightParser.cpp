#include "EnvironmentLightParser.h"
#include "Environment.h"
#include "Logger.h"
#include "SceneLoader.h"

#include "light/EnvironmentLight.h"

#include "DataLisp.h"

#include <algorithm>

namespace PR {
std::shared_ptr<PR::IInfiniteLight> EnvironmentLightParser::parse(Environment* env, const DL::DataGroup& group) const
{
	DL::Data matD = group.getFromKey("material");

	auto light = std::make_shared<EnvironmentLight>();

	if (matD.type() == DL::Data::T_String) {
		if (env->hasMaterial(matD.getString()))
			light->setMaterial(env->getMaterial(matD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find material " << matD.getString() << std::endl;
	}

	return light;
}
} // namespace PR
