#include "Entity.h"

#include <sstream>

#include "Logger.h"
#include "performance/Performance.h"

namespace PR
{
	Entity::Entity(uint32 id, const std::string& name) :
		mName(name), mID(id), mFlags(EF_ScaleLight),
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

		PR_LOGGER.logf(L_Info, M_Entity,"%s: P[%.3f,%.3f,%.3f,%.3f] R[%.3f,%.3f,%.3f] S[%.3f,%.3f,%.3f]",
			mName.c_str(),
			PM::pm_GetX(mPosition), PM::pm_GetY(mPosition), PM::pm_GetZ(mPosition), PM::pm_GetW(mPosition),
			PM::pm_RadToDeg(PM::pm_GetX(angle)), PM::pm_RadToDeg(PM::pm_GetY(angle)), PM::pm_RadToDeg(PM::pm_GetZ(angle)),
			PM::pm_GetX(mScale), PM::pm_GetY(mScale), PM::pm_GetZ(mScale));

		if(std::abs(PM::pm_GetW(mPosition) - 1) > PM_EPSILON)
			PR_LOGGER.logf(L_Warning, M_Entity, "Entity %s using inhomogen coordinates", name().c_str());

		if(PM::pm_MagnitudeSqr3D(mScale) <= PM_EPSILON)
			PR_LOGGER.logf(L_Warning, M_Entity, "Entity %s has zero scale attribute", name().c_str());

		if(std::abs(PM::pm_Determinant4D(matrix())) <= PM_EPSILON)
			PR_LOGGER.logf(L_Warning, M_Entity, "Entity %s has zero determinant matrix", name().c_str());
	}

	std::string Entity::dumpInformation() const
	{
		std::stringstream stream;
		stream << "<Entity> [" << mID << "]: " << std::endl
			<< "  Position:        {" << PM::pm_GetX(mPosition)
			<< "|" << PM::pm_GetY(mPosition)
			<< "|" << PM::pm_GetZ(mPosition) << "}" << std::endl
			<< "  Scale:           {" << PM::pm_GetX(mScale)
			<< "|" << PM::pm_GetY(mScale)
			<< "|" << PM::pm_GetZ(mScale) << "}" << std::endl
			<< "  Rotation:        {" << PM::pm_GetX(mRotation)
			<< "|" << PM::pm_GetY(mRotation)
			<< "|" << PM::pm_GetZ(mRotation)
			<< "|" << PM::pm_GetW(mRotation) << "}" << std::endl
			<< "  Flag&Debug:      " << (mFlags & EF_Debug != 0 ? "true" : "false") << std::endl
			<< "  Flag&LocalArea:  " << (mFlags & EF_LocalArea != 0 ? "true" : "false") << std::endl
			<< "  Flag&ScaleLight: " << (mFlags & EF_ScaleLight != 0 ? "true" : "false") << std::endl;

		return stream.str();
	}
}
