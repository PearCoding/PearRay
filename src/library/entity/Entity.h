#pragma once

#include "PR_Config.h"
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <string>

namespace PR {
enum EntityFlags {
	EF_Debug	  = 0x1,
	EF_LocalArea  = 0x2,
	EF_ScaleLight = 0x4,
};

#define ENTITY_CLASS \
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

class PR_LIB Entity {
public:
	ENTITY_CLASS

	Entity(uint32 id, const std::string& name);
	virtual ~Entity();

	inline uint32 id() const;

	inline void setName(const std::string& name);
	inline std::string name() const;

	virtual std::string type() const;
	virtual bool isRenderable() const;

	inline void setFlags(uint8 f);
	inline uint8 flags() const;

	inline void setPosition(const Eigen::Vector3f& pos);
	inline Eigen::Vector3f position() const;

	inline void setScale(const Eigen::Vector3f& s);
	inline Eigen::Vector3f scale() const;

	inline void setRotation(const Eigen::Quaternionf& quat);
	inline Eigen::Quaternionf rotation() const;

	// FIXME: Unfortunatly we have to disable alignment for the transformations.
	// A SIGSEV is raised in release build otherwise. 
	// It may be a bug in Eigen or somewhere in my own code.
	typedef Eigen::Transform<float, 3, Eigen::Affine, Eigen::DontAlign> Transform;
	inline const Transform& transform() const;
	inline const Transform& invTransform() const;

	/* Matrix to be used by directions/normals */
	inline const Eigen::Matrix3f& directionMatrix() const;
	inline const Eigen::Matrix3f& invDirectionMatrix() const;

	virtual std::string toString() const;

	inline void freeze();
	inline bool isFrozen() const;

	// Events:
	/* The entity will not be changed after this. */
	virtual void onFreeze();

	inline void invalidateCache();

	virtual std::string dumpInformation() const;

private:
	std::string mName;
	uint32 mID;
	uint8 mFlags;

	Eigen::Vector3f mPosition;
	Eigen::Vector3f mScale;
	Eigen::Quaternionf mRotation;

	bool mFrozen;

	mutable bool mReCache;
	mutable Transform mTransformCache;
	mutable Transform mInvTransformCache;
	mutable Eigen::Matrix3f mNormalMatrixCache;
	mutable Eigen::Matrix3f mInvNormalMatrixCache;
};
}

#include "Entity.inl"
