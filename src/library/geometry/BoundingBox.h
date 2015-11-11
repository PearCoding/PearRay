#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class Ray;
	/**
	 * A axis aligned bounding box (AABB)
	 */
	class PR_LIB BoundingBox
	{
	public:
		BoundingBox();
		BoundingBox(const PM::vec3& upperbound, const PM::vec3& lowerbound);
		BoundingBox(float width, float height, float depth);

		BoundingBox(const BoundingBox& other);
		BoundingBox& operator = (const BoundingBox& other);

		PM::vec3 upperBound() const;
		void setUpperBound(const PM::vec3& bound);

		PM::vec3 lowerBound() const;
		void setLowerBound(const PM::vec3& bound);

		PM::vec3 center() const;

		float width() const;
		float height() const;
		float depth() const;
		float volume() const;

		bool isValid() const;

		bool contains(const PM::vec3& point) const;
		bool intersects(const Ray& ray) const;
		bool intersects(const Ray& ray, PM::vec3& collisionPoint) const;

		void put(const PM::vec3& point);
		void combine(const BoundingBox& other);

		BoundingBox putted(const PM::vec3& point) const;
		BoundingBox combined(const BoundingBox& other) const;

	private:
		PM::vec3 mUpperBound;
		PM::vec3 mLowerBound;
	};
}