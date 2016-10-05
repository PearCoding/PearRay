#include "Entity.h"

#include <sstream>

#include "Logger.h"
#include "performance/Performance.h"

namespace PR
{
	Entity::Entity(const std::string& name) :
		mName(name), mFlags(EF_ScaleLight),
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

	void Entity::onFreeze()
	{
		PR_GUARD_PROFILE();

		PM::vec3 angle = PM::pm_RotationQuatToXYZ(mRotation);

		PR_LOGGER.logf(L_Info, M_Camera,"%s: P[%.3f,%.3f,%.3f,%.3f] R[%.3f,%.3f,%.3f] S[%.3f,%.3f,%.3f]",
			mName.c_str(),
			PM::pm_GetX(mPosition), PM::pm_GetY(mPosition), PM::pm_GetZ(mPosition), PM::pm_GetW(mPosition),
			PM::pm_RadToDeg(PM::pm_GetX(angle)), PM::pm_RadToDeg(PM::pm_GetY(angle)), PM::pm_RadToDeg(PM::pm_GetZ(angle)),
			PM::pm_GetX(mScale), PM::pm_GetY(mScale), PM::pm_GetZ(mScale));
	}
}