#pragma once

#include "PR_Config.h"
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <string>

namespace PR
{
	enum EntityFlags
	{
		EF_Debug = 0x1,
		EF_LocalArea = 0x2,
		EF_ScaleLight = 0x4,
	};

	class PR_LIB Entity
	{
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		
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

		inline const Eigen::Affine3f& transform() const;
		inline const Eigen::Affine3f& invTransform() const;

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
		mutable Eigen::Affine3f mTransformCache;
		mutable Eigen::Affine3f mInvTransformCache;
		mutable Eigen::Matrix3f mNormalMatrixCache;
		mutable Eigen::Matrix3f mInvNormalMatrixCache;
	};
}

#include "Entity.inl"
