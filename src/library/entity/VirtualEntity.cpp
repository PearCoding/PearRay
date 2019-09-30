#include "VirtualEntity.h"

#include <sstream>

#include "Logger.h"


namespace PR {
VirtualEntity::VirtualEntity(uint32 id, const std::string& name)
	: IFreezable()
	, mName(name)
	, mID(id)
	, mFlags(0)
	, mTransform(Transform::Identity())
{
}

VirtualEntity::~VirtualEntity()
{
}

void VirtualEntity::onFreeze(RenderContext*)
{
	mInvTransformCache	= mTransform.inverse();
	mNormalMatrixCache	= mTransform.linear().inverse().transpose();
	mInvNormalMatrixCache = mNormalMatrixCache.inverse();

	Eigen::Matrix3f rot;
	Eigen::Matrix3f sca;
	mTransform.computeRotationScaling(&rot, &sca);
	//mTransform.computeScalingRotation(&sca, &rot);

	PR_LOG(L_INFO) << mName << ": P[" << mTransform.translation() << "] R[" << Eigen::Quaternionf(rot) << "] S[" << sca.diagonal() << "]" << std::endl;

	Eigen::Matrix3f irot;
	Eigen::Matrix3f isca;
	mInvTransformCache.computeRotationScaling(&irot, &isca);
	PR_LOG(L_INFO) << " IP[" << mInvTransformCache.translation() << "] IR[" << Eigen::Quaternionf(irot) << "] IS[" << isca.diagonal() << "]" << std::endl;

	if (sca.squaredNorm() <= PR_EPSILON)
		PR_LOG(L_WARNING) << "VirtualEntity " << mName << " has zero scale attribute" << std::endl;
}

std::string VirtualEntity::dumpInformation() const
{
	const auto pos = mTransform.translation();
	Eigen::Matrix3f rot;
	Eigen::Matrix3f sca;
	mTransform.computeRotationScaling(&rot, &sca);

	Eigen::Quaternionf quat(rot);
	Eigen::Vector3f scav = sca.diagonal();

	std::stringstream stream;
	stream << "<VirtualEntity> [" << mID << "]: " << std::endl
		   << "  Position:        {" << pos(0)
		   << "|" << pos(1)
		   << "|" << pos(2) << "}" << std::endl
		   << "  Scale:           {" << scav(0)
		   << "|" << scav(1)
		   << "|" << scav(2) << "}" << std::endl
		   << "  Rotation:        {" << quat.x()
		   << "|" << quat.y()
		   << "|" << quat.z()
		   << "|" << quat.w() << "}" << std::endl
		   << "  Flag&Debug:      " << ((mFlags & EF_Debug) != 0 ? "true" : "false") << std::endl
		   << "  Flag&LocalArea:  " << ((mFlags & EF_LocalArea) != 0 ? "true" : "false") << std::endl;

	return stream.str();
}
}
