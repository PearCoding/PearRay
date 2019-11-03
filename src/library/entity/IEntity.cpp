#include "IEntity.h"
#include "Logger.h"

namespace PR {
IEntity::IEntity(uint32 id, const std::string& name)
	: VirtualEntity(id, name)
	, mContainerID(0)
{
}

IEntity::~IEntity()
{
}

BoundingBox IEntity::calcWorldBoundingBox() const
{
	const BoundingBox bx = localBoundingBox();

	const Eigen::Matrix3f M = transform().linear();
	const Vector3f x1 = M.col(0)*bx.lowerBound()(0);
	const Vector3f x2 = M.col(0)*bx.upperBound()(0);

	const Vector3f y1 = M.col(1)*bx.lowerBound()(1);
	const Vector3f y2 = M.col(1)*bx.upperBound()(1);

	const Vector3f z1 = M.col(2)*bx.lowerBound()(2);
	const Vector3f z2 = M.col(2)*bx.upperBound()(2);

	return BoundingBox(
		x1.array().max(x2.array()).matrix() + y1.array().max(y2.array()).matrix() + z1.array().max(z2.array()).matrix() + transform().translation(),
		x1.array().min(x2.array()).matrix() + y1.array().min(y2.array()).matrix() + z1.array().min(z2.array()).matrix() + transform().translation()
	);
}

void IEntity::onFreeze(RenderContext* context)
{
	VirtualEntity::onFreeze(context);

	mWorldBoundingBox_Cache = calcWorldBoundingBox();

	if (!mWorldBoundingBox_Cache.isValid())
		PR_LOG(L_WARNING) << "Render entity " << name() << " world bounding box is invalid!" << std::endl;
}

std::string IEntity::dumpInformation() const
{
	std::stringstream stream;
	stream << VirtualEntity::dumpInformation()
		   << "  <RenderEntity>: " << std::endl
		   << "    IsLight:       " << (isLight() ? "true" : "false") << std::endl
		   << "    IsCollidable:  " << (isCollidable() ? "true" : "false") << std::endl
		   << "    CollisionCost: " << collisionCost() << std::endl;

	return stream.str();
}
}
