#include "Entity.h"

#include <sstream>

#include "Logger.h"
#include "performance/Performance.h"

namespace PR {
Entity::Entity(uint32 id, const std::string& name)
	: mName(name)
	, mID(id)
	, mFlags(0)
	, mPosition(0, 0, 0)
	, mScale(1, 1, 1)
	, mRotation(Eigen::Quaternionf::Identity())
	, mFrozen(false)
	, mReCache(true)
{
}

Entity::~Entity()
{
}

std::string Entity::toString() const
{
	std::stringstream stream;
	Eigen::Vector3f pos = position();
	stream << name() << "[" << type() << "]{" << pos(0) << "|" << pos(1) << "|" << pos(2) << "}";

	return stream.str();
}

void Entity::onFreeze()
{
	PR_GUARD_PROFILE();

	PR_LOGGER.logf(L_Info, M_Entity, "%s: P[%.3f,%.3f,%.3f] R[%.3f,%.3f,%.3f,%f] S[%.3f,%.3f,%.3f]",
				   mName.c_str(),
				   mPosition(0), mPosition(1), mPosition(2),
				   mRotation.x(), mRotation.y(), mRotation.z(), mRotation.w(),
				   mScale(0), mScale(1), mScale(2));

	if (mScale.squaredNorm() <= PR_EPSILON)
		PR_LOGGER.logf(L_Warning, M_Entity, "Entity %s has zero scale attribute", name().c_str());
}

std::string Entity::dumpInformation() const
{
	std::stringstream stream;
	stream << "<Entity> [" << mID << "]: " << std::endl
		   << "  Position:        {" << mPosition(0)
		   << "|" << mPosition(1)
		   << "|" << mPosition(2) << "}" << std::endl
		   << "  Scale:           {" << mScale(0)
		   << "|" << mScale(1)
		   << "|" << mScale(2) << "}" << std::endl
		   << "  Rotation:        {" << mRotation.x()
		   << "|" << mRotation.y()
		   << "|" << mRotation.z()
		   << "|" << mRotation.w() << "}" << std::endl
		   << "  Flag&Debug:      " << ((mFlags & EF_Debug) != 0 ? "true" : "false") << std::endl
		   << "  Flag&LocalArea:  " << ((mFlags & EF_LocalArea) != 0 ? "true" : "false") << std::endl;

	return stream.str();
}
}
