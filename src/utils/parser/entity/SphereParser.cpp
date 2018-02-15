#include "SphereParser.h"
#include "Environment.h"
#include "Logger.h"
#include "SceneLoader.h"

#include "entity/SphereEntity.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<PR::Entity> SphereParser::parse(Environment* env, const std::string& name,
												const std::string& obj, const DL::DataGroup& group) const
{
	DL::Data materialD = group.getFromKey("material");
	DL::Data radiusD   = group.getFromKey("radius");

	float r = 1;
	if (radiusD.isNumber())
		r = radiusD.getNumber();
	else
		PR_LOGGER.logf(L_Warning, M_Scene, "Entity %s has no radius. Assuming 1.", name.c_str());

	auto sphere = std::make_shared<SphereEntity>(env->scene().entities().size() + 1, name, r);

	if (materialD.type() == DL::Data::T_String) {
		if (env->hasMaterial(materialD.getString()))
			sphere->setMaterial(env->getMaterial(materialD.getString()));
		else
			PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialD.getString().c_str());
	}

	return sphere;
}
} // namespace PR
