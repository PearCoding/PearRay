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

		inline PM::vec3 position() const { return mPosition; }
		inline void setPosition(const PM::vec3& pos) { PM::pm_Copy(mPosition, pos); }

		inline PM::vec3 xAxis() const { return mXAxis; }
		void setXAxis(const PM::vec3& xAxis);

		inline PM::vec3 yAxis() const { return mYAxis; }
		void setYAxis(const PM::vec3& yAxis);

		void setAxis(const PM::vec3& xAxis, const PM::vec3& yAxis);

		inline PM::vec3 normal() const { return mNormal; }
		inline PM::vec3 center() const
		{
			return PM::pm_Add(mPosition, PM::pm_Add(PM::pm_Scale(mXAxis, 0.5f), PM::pm_Scale(mYAxis, 0.5f)));
		}

		inline float width() const { return mWidth; }
		inline float height() const { return mHeight; }
		inline float surfaceArea() const { return width()*height(); }

		inline bool isValid() const
		{
			return PM::pm_MagnitudeSqr3D(mXAxis)*PM::pm_MagnitudeSqr3D(mYAxis) > 0;
		}


		bool contains(const PM::vec3& point) const;
		bool intersects(const Ray& ray, PM::vec3& collisionPoint, float& t, float& u, float& v) const;

		void project(const PM::vec3& point, float& u, float& v) const;

		inline BoundingBox toBoundingBox() const
		{
			BoundingBox box = toLocalBoundingBox();
			box.shift(mPosition);
			return box;
		}
		
		BoundingBox toLocalBoundingBox() const;
	private:
		PM::vec3 mPosition;
		PM::vec3 mXAxis;
		PM::vec3 mYAxis;

		// Cache
		PM::vec3 mNormal;
		float mWidth;
		float mHeight;

		float mWidth2;
		float mHeight2;
	};
}