#pragma once

#include "IObject.h"

#include <string>

namespace PR {
enum EntityFlags {
	EF_Debug	 = 0x1,
	EF_LocalArea = 0x2,
};

#define ENTITY_CLASS \
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

class PR_LIB_CORE ITransformable : public IObject {
public:
	ENTITY_CLASS

	ITransformable(uint32 id, const std::string& name);
	virtual ~ITransformable();

	inline uint32 id() const;

	inline void setName(const std::string& name);
	inline std::string name() const;

	virtual std::string type() const;
	virtual bool isRenderable() const;

	inline void setFlags(uint8 f);
	inline uint8 flags() const;

	// Unfortunately we have to disable alignment for the transformations.
	// A SIGSEV is raised in release build otherwise.
	// It may be a bug in Eigen or somewhere in my own code.
	typedef Eigen::Transform<float, 3, Eigen::Affine, Eigen::DontAlign> Transform;
	inline void setTransform(const Transform& transform);
	inline const Transform& transform() const;
	inline const Transform& invTransform() const;
	inline float volumeScalefactor() const;

	/* Matrix to be used by normals */
	inline const Eigen::Matrix3f& normalMatrix() const;
	inline const Eigen::Matrix3f& invNormalMatrix() const;

	virtual std::string dumpInformation() const;

	// IObject
	virtual void beforeSceneBuild() override;

private:
	std::string mName;
	uint32 mID;
	uint8 mFlags;

	Transform mTransform;

	void cache();
	float mJacobianDeterminant;
	Transform mInvTransformCache;
	Eigen::Matrix3f mNormalMatrixCache;
	Eigen::Matrix3f mInvNormalMatrixCache;
};
} // namespace PR

#include "ITransformable.inl"
