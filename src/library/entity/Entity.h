#pragma once

#include "Config.h"
#include "PearMath.h"

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
		Entity(const std::string& name, Entity* parent = nullptr);
		virtual ~Entity();

		inline void setName(const std::string& name);
		inline std::string name() const;

		virtual std::string type() const;
		virtual bool isRenderable() const;

		inline void setParent(Entity* parent);
		inline Entity* parent() const;
		inline bool isParent(Entity* entity) const;

		inline void setFlags(uint8 f);
		inline uint8 flags() const;

		inline void setPosition(const PM::vec3& pos);
		inline PM::vec3 position() const;
		inline PM::vec3 worldPosition() const;

		inline void setScale(const PM::vec3& s);
		inline PM::vec3 scale() const;
		inline PM::vec3 worldScale() const;

		inline void setRotation(const PM::quat& quat);
		inline PM::quat rotation() const;
		inline PM::quat worldRotation() const;

		inline PM::mat4 matrix() const;
		inline PM::mat4 invMatrix() const;

		inline PM::mat4 worldMatrix() const;
		inline PM::mat4 worldInvMatrix() const;

		/* Matrix to be used by directions/normals */
		inline PM::mat4 directionMatrix() const;
		inline PM::mat4 invDirectionMatrix() const;
		inline PM::mat4 worldDirectionMatrix() const;
		inline PM::mat4 worldInvDirectionMatrix() const;

		virtual std::string toString() const;

		inline void freeze();
		inline bool isFrozen() const;
		
		// Events:
		/* The entity will not be changed after this. */
		virtual void onFreeze();

		inline void invalidateCache();
	private:
		std::string mName;
		Entity* mParent;

		uint8 mFlags;

		PM::vec3 mPosition;
		PM::vec3 mScale;
		PM::quat mRotation;

		bool mFrozen;

		mutable bool mReCache;
		mutable PM::mat mMatrixCache;
		mutable PM::mat mInvMatrixCache;

		PM::vec3 mGlobalPositionCache;
		PM::vec3 mGlobalScaleCache;
		PM::quat mGlobalRotationCache;
		PM::mat mGlobalMatrixCache;
		PM::mat mGlobalInvMatrixCache;
	};
}

#include "Entity.inl"