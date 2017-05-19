#include "RenderEntity.h"

#include "material/Material.h"

#include "performance/Performance.h"

namespace PR
{
	RenderEntity::RenderEntity(uint32 id, const std::string& name) :
		Entity(id, name)
	{
	}

	RenderEntity::~RenderEntity()
	{
	}

	BoundingBox RenderEntity::calcWorldBoundingBox() const
	{
		PR_GUARD_PROFILE();

		const BoundingBox bx = localBoundingBox();

		const Eigen::Vector3f v1 = transform()*
			Eigen::Vector3f(bx.upperBound()(0), bx.upperBound()(1), bx.upperBound()(2));
		const Eigen::Vector3f v2 = transform()*
			Eigen::Vector3f(bx.lowerBound()(0), bx.upperBound()(1), bx.upperBound()(2));
		const Eigen::Vector3f v3 = transform()*
			Eigen::Vector3f(bx.lowerBound()(0), bx.lowerBound()(1), bx.upperBound()(2));
		const Eigen::Vector3f v4 = transform()*
			Eigen::Vector3f(bx.lowerBound()(0), bx.upperBound()(1), bx.lowerBound()(2));
		const Eigen::Vector3f v5 = transform()*
			Eigen::Vector3f(bx.lowerBound()(0), bx.lowerBound()(1), bx.lowerBound()(2));
		const Eigen::Vector3f v6 = transform()*
			Eigen::Vector3f(bx.upperBound()(0), bx.lowerBound()(1), bx.upperBound()(2));
		const Eigen::Vector3f v7 = transform()*
			Eigen::Vector3f(bx.upperBound()(0), bx.lowerBound()(1), bx.lowerBound()(2));
		const Eigen::Vector3f v8 = transform()*
			Eigen::Vector3f(bx.upperBound()(0), bx.upperBound()(1), bx.lowerBound()(2));

		BoundingBox w(v1, v2);
		w.put(v3); w.put(v4); w.put(v5);
		w.put(v6); w.put(v7); w.put(v8);
		return w;
	}

	void RenderEntity::onFreeze()
	{
		Entity::onFreeze();

		mWorldBoundingBox_Cache = calcWorldBoundingBox();

		if(!mWorldBoundingBox_Cache.isValid())
			PR_LOGGER.logf(L_Warning, M_Entity, "Render entity %s world bounding box is invalid!", name().c_str());
	}

	
	std::string RenderEntity::dumpInformation() const
	{
		std::stringstream stream;
		stream << Entity::dumpInformation()
			<< "  <RenderEntity>: " << std::endl
			<< "    IsLight:       " << (isLight() ? "true" : "false") << std::endl
			<< "    IsCollidable:  " << (isCollidable() ? "true" : "false") << std::endl
			<< "    CollisionCost: " << collisionCost() << std::endl;

		return stream.str();
	}
}
