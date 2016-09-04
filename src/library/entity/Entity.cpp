#include "Entity.h"

#include <sstream>

#include "Logger.h"

namespace PR
{
	Entity::Entity(const std::string& name, Entity* parent) :
		mName(name), mParent(parent), mFlags(EF_ScaleLight),
		mPosition(PM::pm_Set(0,0,0,1)), mScale(PM::pm_Set(1,1,1,1)), mRotation(PM::pm_IdentityQuat()),
		mReCache(true), mFrozen(false)
	{
	}

	Entity::~Entity()
	{
	}

	void Entity::setName(const std::string& name)
	{
		mName = name;
	}

	std::string Entity::name() const
	{
		return mName;
	}

	std::string Entity::type() const
	{
		return "null";
	}
	
	bool Entity::isRenderable() const
	{
		return false;
	}

	void Entity::setParent(Entity* parent)
	{
		mParent = parent;
		invalidateCache();
	}

	Entity* Entity::parent() const
	{
		return mParent;
	}

	bool Entity::isParent(Entity* entity) const
	{
		if (!entity)
			return true;

		if (mParent == entity)
			return true;

		return mParent ? mParent->isParent(entity) : false;
	}

	void Entity::setFlags(uint8 f)
	{
		mFlags = f;
	}

	uint8 Entity::flags() const
	{
		return mFlags;
	}

	void Entity::setPosition(const PM::vec3& pos)
	{
		mPosition = pos;
		invalidateCache();
	}

	PM::vec3 Entity::position() const
	{
		return mPosition;
	}

	PM::vec3 Entity::worldPosition() const
	{
		if(mFrozen)
			return mGlobalPositionCache;
		else if (mParent)
			return PM::pm_DecomposeTranslation(worldMatrix());
		else
			return mPosition;
	}

	void Entity::setScale(const PM::vec3& scale)
	{
		mScale = scale;
		invalidateCache();
	}

	PM::vec3 Entity::scale() const
	{
		return mScale;
	}

	PM::vec3 Entity::worldScale() const
	{
		if(mFrozen)
			return mGlobalScaleCache;
		else if (mParent)
			return PM::pm_DecomposeScale(worldMatrix());
		else
			return mScale;
	}

	void Entity::setRotation(const PM::quat& quat)
	{
		mRotation = quat;
		invalidateCache();
	}

	PM::quat Entity::rotation() const
	{
		return mRotation;
	}

	PM::quat Entity::worldRotation() const
	{
		if(mFrozen)
			return mGlobalRotationCache;
		else if (mParent)
			return PM::pm_DecomposeRotation(worldMatrix());
		else
			return mRotation;
	}

	PM::mat4 Entity::matrix() const
	{
		if (mReCache)
		{
			mReCache = false;

			mMatrixCache = PM::pm_Multiply(PM::pm_Translation(mPosition), PM::pm_Multiply(PM::pm_Rotation(mRotation), PM::pm_Scaling(mScale)));
			mInvMatrixCache = PM::pm_Inverse(mMatrixCache);
		}

		return mMatrixCache;
	}

	PM::mat4 Entity::invMatrix() const
	{
		if (mReCache)
			matrix();

		return mInvMatrixCache;
	}

	PM::mat4 Entity::worldMatrix() const
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

	PM::mat4 Entity::worldInvMatrix() const
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

	PM::mat4 Entity::directionMatrix() const
	{
		return PM::pm_Transpose(invMatrix());
	}

	PM::mat4 Entity::invDirectionMatrix() const
	{
		return PM::pm_Transpose(matrix());
	}

	PM::mat4 Entity::worldDirectionMatrix() const
	{
		return PM::pm_Transpose(worldInvMatrix());
	}

	PM::mat4 Entity::worldInvDirectionMatrix() const
	{
		return PM::pm_Transpose(worldMatrix());
	}

	std::string Entity::toString() const
	{
		std::stringstream stream;
		PM::vec3 pos = position();
		stream << name() << "[" << type()
			<< "]{" << PM::pm_GetX(pos) 
			<< "|" << PM::pm_GetY(pos) 
			<< "|" << PM::pm_GetZ(pos) << "}";

		return stream.str();
	}

	void Entity::invalidateCache()
	{
		mReCache = true;
		mFrozen = false;
	}

	void Entity::onPreRender()
	{
		mGlobalMatrixCache = worldMatrix();
		mGlobalInvMatrixCache = worldInvMatrix();
		mGlobalPositionCache = worldPosition();
		mGlobalScaleCache = worldScale();
		mGlobalRotationCache = worldRotation();

		PR_LOGGER.logf(L_Info, M_Scene, "W %f,%f,%f,%f", PM::pm_GetX(mGlobalScaleCache), PM::pm_GetY(mGlobalScaleCache), PM::pm_GetZ(mGlobalScaleCache), PM::pm_GetW(mGlobalScaleCache));
		PR_LOGGER.logf(L_Info, M_Scene, "M %f,%f,%f,%f", PM::pm_GetX(mScale), PM::pm_GetY(mScale), PM::pm_GetZ(mScale), PM::pm_GetW(mScale));

		mFrozen = true;
	}
}