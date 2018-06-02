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
		PR_LOG(L_WARNING) << "Entity " << name << " has no radius. Assuming 1." << std::endl;

	auto sphere = std::make_shared<SphereEntity>(env->sceneFactory().fullEntityCount() + 1, name, r);

	if (materialD.type() == DL::Data::T_String) {
		if (env->hasMaterial(materialD.getString()))
			sphere->setMaterial(env->getMaterial(materialD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find material " << materialD.getString() << std::endl;
	}

	return sphere;
}
} // namespace PR
