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
		Entity(uint32 id, const std::string& name);
		virtual ~Entity();

		inline uint32 id() const;
		
		inline void setName(const std::string& name);
		inline std::string name() const;

		virtual std::string type() const;
		virtual bool isRenderable() const;

		inline void setFlags(uint8 f);
		inline uint8 flags() const;

		inline void setPosition(const PM::vec3& pos);
		inline PM::vec3 position() const;

		inline void setScale(const PM::vec3& s);
		inline PM::vec3 scale() const;

		inline void setRotation(const PM::quat& quat);
		inline PM::quat rotation() const;

		inline PM::mat4 matrix() const;
		inline PM::mat4 invMatrix() const;

		/* Matrix to be used by directions/normals */
		inline PM::mat4 directionMatrix() const;
		inline PM::mat4 invDirectionMatrix() const;

		virtual std::string toString() const;

		inline void freeze();
		inline bool isFrozen() const;
		
		// Events:
		/* The entity will not be changed after this. */
		virtual void onFreeze();

		inline void invalidateCache();
	private:
		std::string mName;
		uint32 mID;
		uint8 mFlags;

		PM::vec3 mPosition;
		PM::vec3 mScale;
		PM::quat mRotation;

		bool mFrozen;

		mutable bool mReCache;
		mutable PM::mat mMatrixCache;
		mutable PM::mat mInvMatrixCache;
	};
}

#include "Entity.inl"