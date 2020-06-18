#include "IEntity.h"
#include "Logger.h"

namespace PR {
IEntity::IEntity(uint32 id, const std::string& name)
	: ITransformable(id, name)
	, mVisibilityFlags(EVF_All)
{
}

IEntity::~IEntity()
{
}

BoundingBox IEntity::calcWorldBoundingBox() const
{
	const BoundingBox bx = localBoundingBox();

	const Eigen::Matrix3f M = transform().linear();
	const Vector3f x1		= M.col(0) * bx.lowerBound()(0);
	const Vector3f x2		= M.col(0) * bx.upperBound()(0);

	const Vector3f y1 = M.col(1) * bx.lowerBound()(1);
	const Vector3f y2 = M.col(1) * bx.upperBound()(1);

	const Vector3f z1 = M.col(2) * bx.lowerBound()(2);
	const Vector3f z2 = M.col(2) * bx.upperBound()(2);

	return BoundingBox(
		x1.array().max(x2.array()).matrix() + y1.array().max(y2.array()).matrix() + z1.array().max(z2.array()).matrix() + transform().translation(),
		x1.array().min(x2.array()).matrix() + y1.array().min(y2.array()).matrix() + z1.array().min(z2.array()).matrix() + transform().translation());
}

void IEntity::beforeSceneBuild()
{
	ITransformable::beforeSceneBuild();

	mWorldBoundingBox_Cache = calcWorldBoundingBox();

	if (mWorldBoundingBox_Cache.isPlanar())
		mWorldBoundingBox_Cache.inflate();

	if (!mWorldBoundingBox_Cache.isValid())
		PR_LOG(L_WARNING) << "Entity " << name() << " world bounding box is invalid!" << std::endl;
}

std::string IEntity::dumpInformation() const
{
	std::stringstream stream;
	stream << ITransformable::dumpInformation()
		   << "  <IEntity>: " << std::endl
		   << "    IsLight:       " << (isLight() ? "true" : "false") << std::endl
		   << "    IsCollidable:  " << (isCollidable() ? "true" : "false") << std::endl
		   << "    IsCameraVisible:  " << ((visibilityFlags() & EVF_Camera) ? "true" : "false") << std::endl
		   << "    IsLightVisible:  " << ((visibilityFlags() & EVF_Light) ? "true" : "false") << std::endl
		   << "    IsBounceVisible:  " << ((visibilityFlags() & EVF_Bounce) ? "true" : "false") << std::endl
		   << "    IsShadowVisible:  " << ((visibilityFlags() & EVF_Shadow) ? "true" : "false") << std::endl
		   << "    CollisionCost: " << collisionCost() << std::endl
		   << "    SurfaceArea:   " << surfaceArea() << std::endl
		   << "    BBVolume:      " << worldBoundingBox().volume() << std::endl;
	;

	return stream.str();
}
} // namespace PR
