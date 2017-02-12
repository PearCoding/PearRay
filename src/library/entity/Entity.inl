#pragma once

namespace PR
{
	inline uint32 Entity::id() const
	{
		return mID;
	}

	inline void Entity::setName(const std::string& name)
	{
		mName = name;
	}

	inline std::string Entity::name() const
	{
		return mName;
	}

	inline std::string Entity::type() const
	{
		return "null";
	}

	inline bool Entity::isRenderable() const
	{
		return false;
	}

	inline void Entity::setFlags(uint8 f)
	{
		mFlags = f;
	}

	inline uint8 Entity::flags() const
	{
		return mFlags;
	}

	inline void Entity::setPosition(const PM::vec3& pos)
	{
		mPosition = pos;
		invalidateCache();
	}

	inline PM::vec3 Entity::position() const
	{
		return mPosition;
	}

	inline void Entity::setScale(const PM::vec3& scale)
	{
		mScale = scale;
		invalidateCache();
	}

	inline PM::vec3 Entity::scale() const
	{
		return mScale;
	}

	inline void Entity::setRotation(const PM::quat& quat)
	{
		mRotation = quat;
		invalidateCache();
	}

	inline PM::quat Entity::rotation() const
	{
		return mRotation;
	}

	inline PM::mat4 Entity::matrix() const
	{
		if (mReCache)
		{
			mReCache = false;

			mMatrixCache = PM::pm_Multiply(PM::pm_Translation(mPosition), PM::pm_Multiply(PM::pm_Rotation(mRotation), PM::pm_Scaling(mScale)));
			mInvMatrixCache = PM::pm_Inverse4D(mMatrixCache);
		}

		return mMatrixCache;
	}

	inline PM::mat4 Entity::invMatrix() const
	{
		if (mReCache)
			matrix();

		return mInvMatrixCache;
	}

	inline PM::mat4 Entity::directionMatrix() const
	{
		return PM::pm_Transpose(invMatrix());
	}

	inline PM::mat4 Entity::invDirectionMatrix() const
	{
		return PM::pm_Transpose(matrix());
	}

	inline void Entity::invalidateCache()
	{
		mReCache = true;
		mFrozen = false;
	}

	inline void Entity::freeze()
	{
		if(!mFrozen)
		{
			onFreeze();
			mFrozen = true;
		}
	}

	inline bool Entity::isFrozen() const
	{
		return mFrozen;
	}
}
