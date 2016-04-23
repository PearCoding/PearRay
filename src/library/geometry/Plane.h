#pragma once

#include "Config.h"
#include "PearMath.h"

#include "geometry/BoundingBox.h"

namespace PR
{
	class Ray;
	class PR_LIB Plane
	{
	public:
		Plane();
		Plane(const PM::vec3& pos, const PM::vec3& xAxis, const PM::vec3& yAxis);
		Plane(float width, float height);

		Plane(const Plane& other);
		Plane& operator = (const Plane& other);

		PM::vec3 position() const;
		void setPosition(const PM::vec3& pos);

		PM::vec3 xAxis() const;
		void setXAxis(const PM::vec3& xAxis);

		PM::vec3 yAxis() const;
		void setYAxis(const PM::vec3& yAxis);

		PM::vec3 normal() const;
		PM::vec3 center() const;

		float width() const;
		float height() const;
		float surface() const;

		bool isValid() const;

		bool contains(const PM::vec3& point) const;
		bool intersects(const Ray& ray) const;
		bool intersects(const Ray& ray, PM::vec3& collisionPoint) const;
		bool intersects(const Ray& ray, PM::vec3& collisionPoint, float& u, float& v) const;

		BoundingBox toBoundingBox() const;
		BoundingBox toLocalBoundingBox() const;
	private:
		PM::vec3 mPosition;
		PM::vec3 mXAxis;
		PM::vec3 mYAxis;

		// Cache
		PM::vec3 mNormal;
		float mWidth;
		float mHeight;
	};
}