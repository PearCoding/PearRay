#include "DistantLightParser.h"
#include "Environment.h"
#include "Logger.h"
#include "SceneLoader.h"

#include "light/DistantLight.h"

#include "DataLisp.h"

#include <algorithm>

namespace PR {
std::shared_ptr<PR::IInfiniteLight> DistantLightParser::parse(Environment* env, const DL::DataGroup& group) const
{
	DL::Data dirD = group.getFromKey("direction");
	DL::Data matD = group.getFromKey("material");

	auto light = std::make_shared<DistantLight>();

	if (matD.type() == DL::Data::T_String) {
		if (env->hasMaterial(matD.getString()))
			light->setMaterial(env->getMaterial(matD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find material " << matD.getString() << std::endl;
	}

	if (dirD.type() == DL::Data::T_Group) {
		bool ok;
		auto v = SceneLoader::getVector(dirD.getGroup(), ok);
		if (ok)
			light->setDirection(v);
		else
			PR_LOG(L_WARNING) << "Invalid direction given." << std::endl;
	}

	return light;
}
} // namespace PR
