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
		   << "    IsLight:          " << (isLight() ? "true" : "false") << std::endl
		   << "    IsCollidable:     " << (isCollidable() ? "true" : "false") << std::endl
		   << "    IsCameraVisible:  " << ((visibilityFlags() & EVF_Camera) ? "true" : "false") << std::endl
		   << "    IsLightVisible:   " << ((visibilityFlags() & EVF_Light) ? "true" : "false") << std::endl
		   << "    IsBounceVisible:  " << ((visibilityFlags() & EVF_Bounce) ? "true" : "false") << std::endl
		   << "    IsShadowVisible:  " << ((visibilityFlags() & EVF_Shadow) ? "true" : "false") << std::endl
		   << "    CollisionCost:    " << collisionCost() << std::endl
		   << "    LocalSurfaceArea: " << localSurfaceArea() << std::endl
		   << "    BBVolume:         " << worldBoundingBox().volume() << std::endl;
	;

	return stream.str();
}
} // namespace PR
