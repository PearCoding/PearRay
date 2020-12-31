#pragma once

#include "PR_Config.h"

#include <string>

namespace PR {
#define ENTITY_CLASS \
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

/// Class representing a transformable. No transform changes allowed however
class PR_LIB_CORE ITransformable {
public:
	ENTITY_CLASS

	ITransformable(const std::string& name, const Transformf& transform);
	virtual ~ITransformable();

	inline std::string name() const;

	virtual std::string type() const;
	virtual bool isRenderable() const;

	inline const Transformf& transform() const;
	inline const Transformf& invTransform() const;
	inline float volumeScalefactor() const;

	/* Matrix to be used by normals */
	inline const Eigen::Matrix3f& normalMatrix() const;
	inline const Eigen::Matrix3f& invNormalMatrix() const;

	virtual std::string dumpInformation() const;

private:
	const std::string mName;

	const Transformf mTransform;
	const Transformf mInvTransformCache;
	const Eigen::Matrix3f mNormalMatrixCache;
	const Eigen::Matrix3f mInvNormalMatrixCache;
	const float mJacobianDeterminant;
};
} // namespace PR

#include "ITransformable.inl"
