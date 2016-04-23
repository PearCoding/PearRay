#include "Plane.h"

#include "ray/Ray.h"

namespace PR
{
	Plane::Plane() :
		mPosition(PM::pm_Set(0, 0, 0, 1)), mXAxis(PM::pm_Set(1, 0, 0, 1)), mYAxis(PM::pm_Set(0, 1, 0, 1)),
		mNormal(PM::pm_Set(0,0,1,1)), mWidth(1), mHeight(1)
	{
	}

	Plane::Plane(const PM::vec3& pos, const PM::vec3& xAxis, const PM::vec3& yAxis) :
		mPosition(pos), mXAxis(xAxis), mYAxis(yAxis)
	{
		mNormal = PM::pm_Negate(PM::pm_Normalize3D(PM::pm_Cross3D(mXAxis, mYAxis)));
		mWidth = PM::pm_Magnitude3D(mXAxis);
		mHeight = PM::pm_Magnitude3D(mYAxis);
	}

	Plane::Plane(float width, float height) :
		mPosition(PM::pm_Set(0,0,0,1)),
		mXAxis(PM::pm_Set(width, 0, 0, 1)),
		mYAxis(PM::pm_Set(0, height, 0, 1)),
		mNormal(PM::pm_Set(0, 0, 1, 1)),
		mWidth(width), mHeight(height)
	{
		PR_ASSERT(width > 0);
		PR_ASSERT(height > 0);
	}

	Plane::Plane(const Plane& other)
	{
		PM::pm_Copy(mPosition, other.position());
		PM::pm_Copy(mXAxis, other.xAxis());
		PM::pm_Copy(mYAxis, other.yAxis());
		PM::pm_Copy(mNormal, other.normal());
		mWidth = other.mWidth;
		mHeight = other.mHeight;
	}

	Plane& Plane::operator = (const Plane& other)
	{
		PM::pm_Copy(mPosition, other.position());
		PM::pm_Copy(mXAxis, other.xAxis());
		PM::pm_Copy(mYAxis, other.yAxis());
		PM::pm_Copy(mNormal, other.normal());
		mWidth = other.mWidth;
		mHeight = other.mHeight;
		return *this;
	}

	PM::vec3 Plane::position() const
	{
		return mPosition;
	}

	void Plane::setPosition(const PM::vec3& pos)
	{
		PM::pm_Copy(mPosition, pos);
	}

	PM::vec3 Plane::xAxis() const
	{
		return mXAxis;
	}

	void Plane::setXAxis(const PM::vec3& v)
	{
		PM::pm_Copy(mXAxis, v);
		mNormal = PM::pm_Negate(PM::pm_Normalize3D(PM::pm_Cross3D(mXAxis, mYAxis)));
		mWidth = PM::pm_Magnitude3D(mXAxis);
	}

	PM::vec3 Plane::yAxis() const
	{
		return mYAxis;
	}

	void Plane::setYAxis(const PM::vec3& v)
	{
		PM::pm_Copy(mYAxis, v);
		mNormal = PM::pm_Negate(PM::pm_Normalize3D(PM::pm_Cross3D(mXAxis, mYAxis)));
		mHeight = PM::pm_Magnitude3D(mYAxis);
	}

	PM::vec3 Plane::normal() const
	{
		return mNormal;
	}

	PM::vec3 Plane::center() const
	{
		return PM::pm_Add(mPosition, PM::pm_Add(PM::pm_Scale(mXAxis, 0.5f), PM::pm_Scale(mYAxis, 0.5f)));
	}

	float Plane::width() const
	{
		return mWidth;
	}

	float Plane::height() const
	{
		return mHeight;
	}

	float Plane::surface() const
	{
		return width()*height();
	}

	bool Plane::isValid() const
	{
		return PM::pm_MagnitudeSqr3D(mXAxis)*PM::pm_MagnitudeSqr3D(mYAxis) > 0;
	}

	BoundingBox Plane::toBoundingBox() const
	{
		return BoundingBox(PM::pm_Add(mPosition, PM::pm_Add(mXAxis, mYAxis)), mPosition);
	}

	BoundingBox Plane::toLocalBoundingBox() const
	{
		return BoundingBox(PM::pm_Add(mXAxis, mYAxis), PM::pm_Set(0,0,0,1));
	}

	bool Plane::contains(const PM::vec3& point) const
	{
		PM::vec3 p = PM::pm_Subtract(point, mPosition);
		if (PM::pm_Dot3D(p, mNormal) <= std::numeric_limits<float>::epsilon())// Is on the plane
		{
			//Now check if it is in the width and height area
			if (PM::pm_IsLess(p, PM::pm_Zero()))
				return false;
			
			float u = PM::pm_Dot3D(mXAxis, p);
			float v = PM::pm_Dot3D(mYAxis, p);

			if (v <= 1 && u <= 1)
				return true;
		}
		return false;
	}

	bool Plane::intersects(const Ray& ray) const
	{
		PM::vec3 tmp = PM::pm_Set(0,0,0,1);
		return intersects(ray, tmp);
	}

	bool Plane::intersects(const Ray& ray, PM::vec3& collisionPoint) const
	{
		float u, v;
		return intersects(ray, collisionPoint, u, v);
	}

	bool Plane::intersects(const Ray& ray, PM::vec3& collisionPoint, float& u, float& v) const
	{
		float ln = PM::pm_Dot3D(ray.direction(), mNormal);
		float pn = PM::pm_Dot3D(PM::pm_Subtract(mPosition, ray.startPosition()), mNormal);

		if (std::abs(ln) <= std::numeric_limits<float>::epsilon())//Parallel or on the plane
		{
			return false;
			//if (pn <= std::numeric_limits<float>::epsilon())// Is on the plane!
			//{
			//	// TODO: Should we make this case special?
			//}
		}
		else
		{
			float t = pn / ln;

			if (t < 0)
			{
				return false;
			}
			else
			{
				collisionPoint = PM::pm_SetW(PM::pm_Add(ray.startPosition(), PM::pm_Scale(ray.direction(), t)), 1);
				PM::vec3 p = PM::pm_Subtract(collisionPoint, mPosition);
				u = PM::pm_Dot3D(mXAxis, p) / width();
				v = PM::pm_Dot3D(mYAxis, p) / height();

				if (v >= 0 && v <= 1 && u >= 0 && u <= 1)
					return true;
			}
			return false;
		}
	}
}