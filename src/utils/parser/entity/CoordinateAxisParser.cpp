#include "CoordinateAxisParser.h"
#include "SceneLoader.h"
#include "Environment.h"

#include "Logger.h"

#include "entity/CoordinateAxisEntity.h"

#include "DataLisp.h"

using namespace PR;
namespace PRU
{
	std::shared_ptr<PR::Entity> CoordinateAxisParser::parse(SceneLoader* loader, Environment* env, const std::string& name,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data axisLengthD = group.getFromKey("axis_length");
		DL::Data axisThicknessD = group.getFromKey("axis_thickness");

		DL::Data materialXD = group.getFromKey("x_material");
		DL::Data materialYD = group.getFromKey("y_material");
		DL::Data materialZD = group.getFromKey("z_material");

		auto bnd = std::make_shared<CoordinateAxisEntity>(env->scene().entities().size()+1, name);

		if (axisLengthD.isNumber())
			bnd->setAxisLength(std::max(PM_EPSILON, axisLengthD.getNumber()));

		if (axisThicknessD.isNumber())
			bnd->setAxisThickness(std::max(PM_EPSILON, axisThicknessD.getNumber()));

		if (materialXD.type() == DL::Data::T_String)
		{
			if (env->hasMaterial(materialXD.getString()))
				bnd->setXMaterial(env->getMaterial(materialXD.getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialXD.getString().c_str());
		}

		if (materialYD.type() == DL::Data::T_String)
		{
			if (env->hasMaterial(materialYD.getString()))
				bnd->setYMaterial(env->getMaterial(materialYD.getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialYD.getString().c_str());
		}

		if (materialZD.type() == DL::Data::T_String)
		{
			if (env->hasMaterial(materialZD.getString()))
				bnd->setZMaterial(env->getMaterial(materialZD.getString()));
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Couldn't find material %s.", materialZD.getString().c_str());
		}

		return bnd;
	}
}