#include "Entity.h"

#include <sstream>

namespace PR
{
	Entity::Entity(const std::string& name, Entity* parent) :
		mName(name), mParent(parent),
		mPosition(PM::pm_Set(0,0,0,1)), mScale(PM::pm_Set(1,1,1,1)), mRotation(PM::pm_IdentityQuat()),
		mReCache(true)
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

	void Entity::setParent(Entity* parent)
	{
		mParent = parent;
		mReCache = true;
	}

	Entity* Entity::parent() const
	{
		return mParent;
	}

	void Entity::setPosition(const PM::vec3& pos)
	{
		mPosition = pos;
		mReCache = true;
	}

	PM::vec3 Entity::position() const
	{
		if (mParent)
		{
			return PM::pm_Multiply(mParent->matrix(), mPosition);
		}
		else
		{
			return mPosition;
		}
	}

	void Entity::setScale(const PM::vec3& scale)
	{
		mScale = scale;
		mReCache = true;
	}

	PM::vec3 Entity::scale() const
	{
		if (mParent)
		{
			return PM::pm_Multiply(mParent->scale(), mScale);
		}
		else
		{
			return mScale;
		}
	}

	void Entity::setRotation(const PM::quat& quat)
	{
		mRotation = quat;
		mReCache = true;
	}

	PM::quat Entity::rotation() const
	{
		if (mParent)
		{
			return PM::pm_MultiplyQuat(mParent->rotation(), mRotation);
		}
		else
		{
			return mRotation;
		}
	}

	PM::mat4 Entity::matrix() const
	{
		if (mReCache)
		{
			mReCache = false;

			PM::mat4 m = PM::pm_Multiply(PM::pm_Translation(mPosition),
				PM::pm_Multiply(PM::pm_Rotation(mRotation), PM::pm_Scaling(mScale)));
			if (mParent)
			{
				mMatrixCache = PM::pm_Multiply(mParent->matrix(), m);
			}
			else
			{
				mMatrixCache = m;
			}
		}

		return mMatrixCache;
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
}