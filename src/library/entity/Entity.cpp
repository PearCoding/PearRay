#include "Entity.h"

#include <sstream>

#include "Logger.h"
#include "performance/Performance.h"

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
		PR_GUARD_PROFILE();

		mGlobalMatrixCache = worldMatrix();
		mGlobalInvMatrixCache = worldInvMatrix();
		mGlobalPositionCache = worldPosition();
		mGlobalScaleCache = worldScale();
		mGlobalRotationCache = worldRotation();

		PM::vec3 globalAngle = PM::pm_RotationQuatToXYZ(mGlobalRotationCache);
		PM::vec3 localAngle = PM::pm_RotationQuatToXYZ(mRotation);

		PR_LOGGER.logf(L_Info, M_Camera,"%s: W P[%.3f,%.3f,%.3f,%.3f] R[%.3f,%.3f,%.3f] S[%.3f,%.3f,%.3f]",
			mName.c_str(),
			PM::pm_GetX(mGlobalPositionCache), PM::pm_GetY(mGlobalPositionCache), PM::pm_GetZ(mGlobalPositionCache), PM::pm_GetW(mGlobalPositionCache),
			PM::pm_RadToDeg(PM::pm_GetX(globalAngle)), PM::pm_RadToDeg(PM::pm_GetY(globalAngle)), PM::pm_RadToDeg(PM::pm_GetZ(globalAngle)),
			PM::pm_GetX(mGlobalScaleCache), PM::pm_GetY(mGlobalScaleCache), PM::pm_GetZ(mGlobalScaleCache));

		PR_LOGGER.logf(L_Info, M_Camera,"%s: L P[%.3f,%.3f,%.3f,%.3f] R[%.3f,%.3f,%.3f] S[%.3f,%.3f,%.3f]",
			mName.c_str(),
			PM::pm_GetX(mPosition), PM::pm_GetY(mPosition), PM::pm_GetZ(mPosition), PM::pm_GetW(mPosition),
			PM::pm_RadToDeg(PM::pm_GetX(localAngle)), PM::pm_RadToDeg(PM::pm_GetY(localAngle)), PM::pm_RadToDeg(PM::pm_GetZ(localAngle)),
			PM::pm_GetX(mScale), PM::pm_GetY(mScale), PM::pm_GetZ(mScale));

		mFrozen = true;
	}
}