#pragma once

#include "IFreezable.h"

#include <string>

namespace PR {
enum EntityFlags {
	EF_Debug	 = 0x1,
	EF_LocalArea = 0x2,
};

#define ENTITY_CLASS \
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

class RenderContext;
class PR_LIB Entity : public IFreezable {
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

	// FIXME: Unfortunatly we have to disable alignment for the transformations.
	// A SIGSEV is raised in release build otherwise.
	// It may be a bug in Eigen or somewhere in my own code.
	typedef Eigen::Transform<float, 3, Eigen::Affine, Eigen::DontAlign> Transform;
	inline void setTransform(const Transform& transform);
	inline const Transform& transform() const;
	inline const Transform& invTransform() const;

	/* Matrix to be used by directions/normals */
	inline const Eigen::Matrix3f& directionMatrix() const;
	inline const Eigen::Matrix3f& invDirectionMatrix() const;

	virtual std::string dumpInformation() const;

protected:
	virtual void onFreeze(RenderContext* context) override;

private:
	std::string mName;
	uint32 mID;
	uint8 mFlags;

	Transform mTransform;

	Transform mInvTransformCache;
	Eigen::Matrix3f mNormalMatrixCache;
	Eigen::Matrix3f mInvNormalMatrixCache;
};
} // namespace PR

#include "Entity.inl"
