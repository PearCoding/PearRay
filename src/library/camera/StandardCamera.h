#pragma once

#include "Camera.h"

namespace PR
{
	class PR_LIB StandardCamera : public Camera
	{
	public:
		StandardCamera(const std::string& name, Entity* parent = nullptr);
		virtual ~StandardCamera();

		virtual std::string type() const;

		void setWithAngle(float foh, float fov);
		void setWithSize(float width, float height);

		void setOrthographic(bool b);
		bool isOrthographic() const;

		void setWidth(float w);
		float width() const;

		void setHeight(float h);
		float height() const;

		void setLocalDirection(const PM::vec3& d);
		PM::vec3 localDirection() const;

		void setLocalRight(const PM::vec3& d);
		PM::vec3 localRight() const;

		void setLocalUp(const PM::vec3& d);
		PM::vec3 localUp() const;

		// Depth of Field
		void setFStop(float f);
		float fstop() const;

		void setApertureRadius(float f);
		float apertureRadius() const;

		Ray constructRay(float nx, float ny, float rx, float ry, float t) const;

		// Entity
		void onPreRender() override;// Cache
	private:
		bool mOrthographic;
		float mWidth;
		float mHeight;

		float mFStop;
		float mApertureRadius;

		PM::vec3 mLocalDirection;
		PM::vec3 mLocalRight;
		PM::vec3 mLocalUp;

		// Cache:
		PM::vec3 mDirection_Cache;
		PM::vec3 mRight_Cache;
		PM::vec3 mUp_Cache;

		float mFocalDistance_Cache;
		PM::vec3 mXApertureRadius_Cache;
		PM::vec3 mYApertureRadius_Cache;
	};
}