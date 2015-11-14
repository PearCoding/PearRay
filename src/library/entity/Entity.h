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

		void setParent(Entity* parent);
		Entity* parent() const;

		void setPosition(const PM::vec3& pos);
		PM::vec3 position() const;

		void setScale(const PM::vec3& scale);
		PM::vec3 scale() const;

		void setRotation(const PM::quat& quat);
		PM::quat rotation() const;

		PM::mat4 matrix() const;

		virtual std::string toString() const;
	private:
		std::string mName;
		Entity* mParent;

		PM::vec3 mPosition;
		PM::vec3 mScale;
		PM::quat mRotation;

		mutable bool mReCache;
		mutable PM::mat mMatrixCache;
	};
}