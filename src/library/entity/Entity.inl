#pragma once

namespace PR
{
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

	inline void Entity::setParent(Entity* parent)
	{
		mParent = parent;
		invalidateCache();
	}

	inline Entity* Entity::parent() const
	{
		return mParent;
	}

	inline bool Entity::isParent(Entity* entity) const
	{
		if (!entity)
			return true;

		if (mParent == entity)
			return true;

		return mParent ? mParent->isParent(entity) : false;
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

	inline PM::vec3 Entity::worldPosition() const
	{
		if(mFrozen)
			return mGlobalPositionCache;
		else if (mParent)
			return PM::pm_DecomposeTranslation(worldMatrix());
		else
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

	inline PM::vec3 Entity::worldScale() const
	{
		if(mFrozen)
			return mGlobalScaleCache;
		else if (mParent)
			return PM::pm_DecomposeScale(worldMatrix());
		else
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

	inline PM::quat Entity::worldRotation() const
	{
		if(mFrozen)
			return mGlobalRotationCache;
		else if (mParent)
			return PM::pm_DecomposeRotation(worldMatrix());
		else
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

	inline PM::mat4 Entity::worldMatrix() const
	{
		if(mFrozen)
			return mGlobalMatrixCache;

		if (mReCache)
			matrix();

		if (mParent)
			return PM::pm_Multiply(mParent->matrix(), mMatrixCache);
		else
			return mMatrixCache;
	}

	inline PM::mat4 Entity::worldInvMatrix() const
	{
		if(mFrozen)
			return mGlobalInvMatrixCache;

		if (mReCache)
			matrix();

		if (mParent)
			return PM::pm_Multiply(mInvMatrixCache, mParent->invMatrix());
		else
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

	inline PM::mat4 Entity::worldDirectionMatrix() const
	{
		return PM::pm_Transpose(worldInvMatrix());
	}

	inline PM::mat4 Entity::worldInvDirectionMatrix() const
	{
		return PM::pm_Transpose(worldMatrix());
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