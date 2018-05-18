#include "RenderEntity.h"

#include "material/Material.h"

#include "performance/Performance.h"

namespace PR {
RenderEntity::RenderEntity(uint32 id, const std::string& name)
	: Entity(id, name)
	, mContainerID(0)
{
}

RenderEntity::~RenderEntity()
{
}

BoundingBox RenderEntity::calcWorldBoundingBox() const
{
	PR_GUARD_PROFILE();

	const BoundingBox bx = localBoundingBox();

	const Eigen::Matrix3f M = transform().linear();
	const Eigen::Vector3f x1 = M.col(0)*bx.lowerBound()(0);
	const Eigen::Vector3f x2 = M.col(0)*bx.upperBound()(0);

	const Eigen::Vector3f y1 = M.col(1)*bx.lowerBound()(1);
	const Eigen::Vector3f y2 = M.col(1)*bx.upperBound()(1);

	const Eigen::Vector3f z1 = M.col(2)*bx.lowerBound()(2);
	const Eigen::Vector3f z2 = M.col(2)*bx.upperBound()(2);

	return BoundingBox(
		x1.array().max(x2.array()).matrix() + y1.array().max(y2.array()).matrix() + z1.array().max(z2.array()).matrix() + transform().translation(),
		x1.array().min(x2.array()).matrix() + y1.array().min(y2.array()).matrix() + z1.array().min(z2.array()).matrix() + transform().translation()
	);

	/*const Eigen::Vector3f v1 = transform()
							   * Eigen::Vector3f(bx.upperBound()(0), bx.upperBound()(1), bx.upperBound()(2));
	const Eigen::Vector3f v2 = transform()
							   * Eigen::Vector3f(bx.lowerBound()(0), bx.upperBound()(1), bx.upperBound()(2));
	const Eigen::Vector3f v3 = transform()
							   * Eigen::Vector3f(bx.lowerBound()(0), bx.lowerBound()(1), bx.upperBound()(2));
	const Eigen::Vector3f v4 = transform()
							   * Eigen::Vector3f(bx.lowerBound()(0), bx.upperBound()(1), bx.lowerBound()(2));
	const Eigen::Vector3f v5 = transform()
							   * Eigen::Vector3f(bx.lowerBound()(0), bx.lowerBound()(1), bx.lowerBound()(2));
	const Eigen::Vector3f v6 = transform()
							   * Eigen::Vector3f(bx.upperBound()(0), bx.lowerBound()(1), bx.upperBound()(2));
	const Eigen::Vector3f v7 = transform()
							   * Eigen::Vector3f(bx.upperBound()(0), bx.lowerBound()(1), bx.lowerBound()(2));
	const Eigen::Vector3f v8 = transform()
							   * Eigen::Vector3f(bx.upperBound()(0), bx.upperBound()(1), bx.lowerBound()(2));

	BoundingBox w(v1, v2);
	w.combine(v3);
	w.combine(v4);
	w.combine(v5);
	w.combine(v6);
	w.combine(v7);
	w.combine(v8);
	return w;*/
}

void RenderEntity::onFreeze(RenderContext* context)
{
	Entity::onFreeze(context);

	mWorldBoundingBox_Cache = calcWorldBoundingBox();

	if (!mWorldBoundingBox_Cache.isValid())
		PR_LOG(L_WARNING) << "Render entity " << name() << " world bounding box is invalid!" << std::endl;
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
