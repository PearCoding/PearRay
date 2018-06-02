#include "MeshParser.h"
#include "Environment.h"
#include "Logger.h"
#include "SceneLoader.h"

#include "entity/MeshEntity.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<PR::Entity> MeshParser::parse(Environment* env, const std::string& name,
											  const std::string& obj, const DL::DataGroup& group) const
{
	DL::Data materialsD = group.getFromKey("materials");
	DL::Data meshD		= group.getFromKey("mesh");

	auto me = std::make_shared<MeshEntity>(env->sceneFactory().fullEntityCount() + 1, name);

	if (meshD.type() == DL::Data::T_String) {
		if (env->hasMesh(meshD.getString())) {
			me->setMesh(env->getMesh(meshD.getString()));
		} else {
			PR_LOG(L_WARNING) << "Couldn't find mesh " << meshD.getString() << std::endl;
			return nullptr;
		}
	} else {
		PR_LOG(L_ERROR) << "Invalid mesh entry found." << std::endl;
		return nullptr;
	}

	if (materialsD.type() == DL::Data::T_String) {
		me->reserveMaterialSlots(1);
		if (env->hasMaterial(materialsD.getString()))
			me->setMaterial(0, env->getMaterial(materialsD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find material " << materialsD.getString() << std::endl;
	} else if (materialsD.type() == DL::Data::T_Group) {
		const DL::DataGroup& group = materialsD.getGroup();
		me->reserveMaterialSlots(group.anonymousCount());

		uint32 slot = 0;
		for (uint32 i = 0; i < group.anonymousCount(); ++i) {
			DL::Data matD = group.at(i);
			if (matD.type() == DL::Data::T_String) {
				if (env->hasMaterial(matD.getString()))
					me->setMaterial(slot, env->getMaterial(matD.getString()));
				else
					PR_LOG(L_WARNING) << "Couldn't find material " << matD.getString() << std::endl;

				++slot;
			}
		}
	}

	return me;
}
} // namespace PR
