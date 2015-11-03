#pragma once

#include "Config.h"
#include "geometry/BoundingBox.h"

#include <string>

namespace PR
{
	class FacePoint;
	class Ray;
	class Renderer;
	class Entity
	{
	public:
		Entity(const std::string& name, Entity* parent = nullptr);
		virtual ~Entity();

		void setName(const std::string& name);
		std::string name() const;

		void setParent(Entity* parent);
		Entity* parent() const;

		void setPosition(const PM::vec3& pos);
		PM::vec3 position() const;

		void setScale(const PM::vec3& scale);
		PM::vec3 scale() const;

		void setRotation(const PM::quat& quat);
		PM::quat rotation() const;

		PM::mat4 matrix() const;

		virtual bool isCollidable() const;
		virtual BoundingBox boundingBox() const;
		virtual bool checkCollision(const Ray& ray, FacePoint& collisionPoint);

		virtual void apply(Ray& in, const FacePoint& point, Renderer* renderer);

	private:
		std::string mName;
		Entity* mParent;

		PM::vec3 mPosition;
		PM::vec3 mScale;
		PM::quat mRotation;
	};
}