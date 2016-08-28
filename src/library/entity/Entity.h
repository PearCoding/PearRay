#pragma once

#include "Config.h"
#include "PearMath.h"

#include <string>

namespace PR
{
	class PR_LIB Entity
	{
	public:
		Entity(const std::string& name, Entity* parent = nullptr);
		virtual ~Entity();

		void setName(const std::string& name);
		std::string name() const;

		virtual std::string type() const;
		virtual bool isRenderable() const;

		void setParent(Entity* parent);
		Entity* parent() const;
		bool isParent(Entity* entity) const;

		void enableDebug(bool b);
		bool isDebug() const;

		void setPosition(const PM::vec3& pos);
		PM::vec3 position(bool local = false) const;

		void setScale(const PM::vec3& s);
		PM::vec3 scale(bool local = false) const;

		void setRotation(const PM::quat& quat);
		PM::quat rotation(bool local = false) const;

		PM::mat4 matrix(bool local = false) const;
		PM::mat4 invMatrix(bool local = false) const;

		virtual std::string toString() const;

		// Events:
		/* The entity will not be changed after this. */
		virtual void onPreRender();
		void invalidateCache();

	private:
		std::string mName;
		Entity* mParent;

		bool mDebug;

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