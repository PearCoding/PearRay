#include "Entity.h"

#include <sstream>

#include "Logger.h"

namespace PR
{
	Entity::Entity(const std::string& name, Entity* parent) :
		mName(name), mParent(parent), mFlags(EF_ScaleLight),
		mPosition(PM::pm_Set(0,0,0,1)), mScale(PM::pm_Set(1,1,1,1)), mRotation(PM::pm_IdentityQuat()),
		mFrozen(false), mReCache(true)
	{
	}

	Entity::~Entity()
	{
	}

	std::string Entity::toString() const
	{
		std::stringstream stream;
		PM::vec3 pos = position();
		stream << name() << "[" << type()
			<< "]{" << PM::pm_GetX(pos) 
			<< "|" << PM::pm_GetY(pos) 
			<< "|" << PM::pm_GetZ(pos) << "}";

		return stream.str();
	}

	void Entity::onPreRender()
	{
		mGlobalMatrixCache = worldMatrix();
		mGlobalInvMatrixCache = worldInvMatrix();
		mGlobalPositionCache = worldPosition();
		mGlobalScaleCache = worldScale();
		mGlobalRotationCache = worldRotation();

		PR_LOGGER.logf(L_Info, M_Scene, "W %f,%f,%f,%f", PM::pm_GetX(mGlobalScaleCache), PM::pm_GetY(mGlobalScaleCache), PM::pm_GetZ(mGlobalScaleCache), PM::pm_GetW(mGlobalScaleCache));
		PR_LOGGER.logf(L_Info, M_Scene, "M %f,%f,%f,%f", PM::pm_GetX(mScale), PM::pm_GetY(mScale), PM::pm_GetZ(mScale), PM::pm_GetW(mScale));

		mFrozen = true;
	}
}