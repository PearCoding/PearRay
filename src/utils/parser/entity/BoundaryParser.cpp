#include "BoundaryParser.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "Logger.h"

#include "entity/BoundaryEntity.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<PR::Entity> BoundaryParser::parse(Environment* env, const std::string& name,
												  const std::string& obj, const DL::DataGroup& group) const
{
	DL::Data materialD = group.getFromKey("material");
	DL::Data sizeD	 = group.getFromKey("size");

	Eigen::Vector3f size(1, 1, 1);

	if (sizeD.type() == DL::Data::T_Group) {
		bool ok;
		size = SceneLoader::getVector(sizeD.getGroup(), ok);

		if (!ok) {
			size = Eigen::Vector3f(1, 1, 1);
			PR_LOG(L_WARNING) << "Entity " << name << " has invalid size. Assuming unit cube." << std::endl;
		}
	} else if (sizeD.isNumber()) {
		size = Eigen::Vector3f(sizeD.getNumber(), sizeD.getNumber(), sizeD.getNumber());
	} else {
		PR_LOG(L_WARNING) << "Entity " << name << " has no size. Assuming unit cube." << std::endl;
	}

	auto bnd = std::make_shared<BoundaryEntity>(env->sceneFactory().fullEntityCount() + 1, name,
												BoundingBox(size(0), size(1), size(2)));

	if (materialD.type() == DL::Data::T_String) {
		if (env->hasMaterial(materialD.getString()))
			bnd->setMaterial(env->getMaterial(materialD.getString()));
		else
			PR_LOG(L_WARNING) << "Couldn't find material " << materialD.getString() << "." << std::endl;
	}

	return bnd;
}
} // namespace PR
