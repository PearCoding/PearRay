#include "ITransformable.h"

#include <sstream>

#include "Logger.h"

namespace PR {
ITransformable::ITransformable(uint32 id, const std::string& name)
	: IObject()
	, mName(name)
	, mID(id)
	, mFlags(0)
	, mTransform(Transform::Identity())
{
	cache();
}

ITransformable::~ITransformable()
{
}

void ITransformable::beforeSceneBuild()
{
	IObject::beforeSceneBuild();

	Eigen::Matrix3f rot;
	Eigen::Matrix3f sca;
	mTransform.computeRotationScaling(&rot, &sca);
	//mTransform.computeScalingRotation(&sca, &rot);

	PR_LOG(L_INFO) << mName << ": P" << PR_FMT_MAT(mTransform.translation())
				   << " R" << PR_FMT_MAT(Eigen::Quaternionf(rot))
				   << " S" << PR_FMT_MAT(sca.diagonal()) << std::endl;

	Eigen::Matrix3f irot;
	Eigen::Matrix3f isca;
	mInvTransformCache.computeRotationScaling(&irot, &isca);
	PR_LOG(L_INFO) << " IP" << PR_FMT_MAT(mInvTransformCache.translation())
				   << " IR" << PR_FMT_MAT(Eigen::Quaternionf(irot))
				   << " IS" << PR_FMT_MAT(isca.diagonal()) << std::endl;

	if (sca.squaredNorm() <= PR_EPSILON)
		PR_LOG(L_WARNING) << "ITransformable " << mName << " has zero scale attribute" << std::endl;

	if (std::abs((sca * isca).sum() - 3) > PR_EPSILON)
		PR_LOG(L_WARNING) << "ITransformable " << mName << " scale and inverse scale do not match" << std::endl;
}

void ITransformable::cache()
{
	mInvTransformCache	= mTransform.inverse();
	mNormalMatrixCache	= mTransform.linear().inverse().transpose();
	mInvNormalMatrixCache = mNormalMatrixCache.inverse();
}

std::string ITransformable::dumpInformation() const
{
	const Vector3f pos = mTransform.translation();
	Eigen::Matrix3f rot;
	Eigen::Matrix3f sca;
	mTransform.computeRotationScaling(&rot, &sca);

	Eigen::Quaternionf quat(rot);
	Vector3f scav = sca.diagonal();

	std::stringstream stream;
	stream << "<ITransformable> [" << mID << "]: " << std::endl
		   << "  Position:        " << PR_FMT_MAT(pos) << std::endl
		   << "  Scale:           " << PR_FMT_MAT(scav) << std::endl
		   << "  Rotation:        " << PR_FMT_MAT(quat) << std::endl
		   << "  Flag&Debug:      " << ((mFlags & EF_Debug) != 0 ? "true" : "false") << std::endl
		   << "  Flag&LocalArea:  " << ((mFlags & EF_LocalArea) != 0 ? "true" : "false") << std::endl;

	return stream.str();
}
} // namespace PR
