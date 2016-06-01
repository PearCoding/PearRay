#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class Ray;
	class PR_LIB Sphere
	{
	public:
		Sphere();
		Sphere(PM::vec3 pos, float radius);

		Sphere(const Sphere& other);
		Sphere& operator = (const Sphere& other);

		PM::vec3 position() const;
		void setPosition(const PM::vec3& pos);

		float radius() const;
		void setRadius(float f);

		float surface() const;
		float volume() const;

		bool isValid() const;

		bool contains(const PM::vec3& point) const;
		bool intersects(const Ray& ray, PM::vec3& collisionPoint, float& t) const;

		void put(const PM::vec3& point);
		void combine(const Sphere& other);

		Sphere putted(const PM::vec3& point) const;
		Sphere combined(const Sphere& other) const;

	private:
		PM::vec3 mPosition;
		float mRadius;
	};
}