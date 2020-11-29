#include "ITransformable.h"

#include <sstream>

#include "Logger.h"

namespace PR {
ITransformable::ITransformable(const std::string& name, const Transformf& transform)
	: mName(name)
	, mFlags(0)
	, mTransform(transform)
	, mInvTransformCache(mTransform.inverse())
	, mNormalMatrixCache(mTransform.linear().inverse().transpose())
	, mInvNormalMatrixCache(mNormalMatrixCache.inverse())
	, mJacobianDeterminant(std::abs(mTransform.linear().determinant()))
{
}

ITransformable::~ITransformable()
{
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
	stream << "<ITransformable> [" << mName << "]: " << std::endl
		   << "  Position:        " << PR_FMT_MAT(pos) << std::endl
		   << "  Scale:           " << PR_FMT_MAT(scav) << std::endl
		   << "  Rotation:        " << PR_FMT_MAT(quat) << std::endl
		   << "  Flag&Debug:      " << ((mFlags & EF_Debug) != 0 ? "true" : "false") << std::endl
		   << "  Flag&LocalArea:  " << ((mFlags & EF_LocalArea) != 0 ? "true" : "false") << std::endl;

	return stream.str();
}
} // namespace PR
