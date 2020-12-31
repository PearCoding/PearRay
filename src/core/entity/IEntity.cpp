#include "IEntity.h"
#include "Logger.h"

namespace PR {
IEntity::IEntity(uint32 emission_id, const std::string& name, const Transformf& transform)
	: ITransformable(name, transform)
	, mVisibilityFlags((uint8)EntityVisibility::All)
	, mEmissionID(emission_id)
{
}

IEntity::~IEntity()
{
}

std::string IEntity::dumpInformation() const
{
	std::stringstream stream;
	stream << ITransformable::dumpInformation()
		   << "  <IEntity>: " << std::endl
		   << "    HasEmission:      " << (hasEmission() ? "true" : "false") << std::endl
		   << "    IsCollidable:     " << (isCollidable() ? "true" : "false") << std::endl
		   << "    IsCameraVisible:  " << ((visibilityFlags() & EntityVisibility::Camera) ? "true" : "false") << std::endl
		   << "    IsLightVisible:   " << ((visibilityFlags() & EntityVisibility::Light) ? "true" : "false") << std::endl
		   << "    IsBounceVisible:  " << ((visibilityFlags() & EntityVisibility::Bounce) ? "true" : "false") << std::endl
		   << "    IsShadowVisible:  " << ((visibilityFlags() & EntityVisibility::Shadow) ? "true" : "false") << std::endl
		   << "    CollisionCost:    " << collisionCost() << std::endl
		   << "    LocalSurfaceArea: " << localSurfaceArea() << std::endl
		   << "    BBVolume:         " << worldBoundingBox().volume() << std::endl;
	;

	return stream.str();
}
} // namespace PR
