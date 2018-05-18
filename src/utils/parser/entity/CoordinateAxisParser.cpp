#include "CoordinateAxisParser.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "Logger.h"

#include "entity/CoordinateAxisEntity.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<PR::Entity> CoordinateAxisParser::parse(Environment* env, const std::string& name,
														const std::string& obj, const DL::DataGroup& group) const
{
	DL::Data axisLengthD	= group.getFromKey("axis_length");
	DL::Data axisThicknessD = group.getFromKey("axis_thickness");

	DL::Data materialXD = group.getFromKey("x_material");
	DL::Data materialYD = group.getFromKey("y_material");
	DL::Data materialZD = group.getFromKey("z_material");

	auto bnd = std::make_shared<CoordinateAxisEntity>(env->sceneFactory().entities().size() + 1, name);

	if (axisLengthD.isNumber())
		bnd->setAxisLength(std::max(PR_EPSILON, axisLengthD.getNumber()));

	if (axisThicknessD.isNumber())
		bnd->setAxisThickness(std::max(PR_EPSILON, axisThicknessD.getNumber()));

	if (materialXD.type() == DL::Data::T_String) {
		if (env->hasMaterial(materialXD.getString()))
			bnd->setXMaterial(env->getMaterial(materialXD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find material " << materialXD.getString() << std::endl;
	}

	if (materialYD.type() == DL::Data::T_String) {
		if (env->hasMaterial(materialYD.getString()))
			bnd->setYMaterial(env->getMaterial(materialYD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find material " << materialYD.getString() << std::endl;
	}

	if (materialZD.type() == DL::Data::T_String) {
		if (env->hasMaterial(materialZD.getString()))
			bnd->setZMaterial(env->getMaterial(materialZD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find material " << materialZD.getString() << std::endl;
	}

	return bnd;
}
} // namespace PR
