#pragma once

#include "Config.h"

#include <string>
#include <list>

namespace PR
{
	class Camera
	{
	public:
		Camera(uint32 width, uint32 height, float vangle, float hangle);
		virtual ~Camera();

		void setWidth(uint32 w);
		uint32 width() const;

		void setHeight(uint32 h);
		uint32 height() const;

		void setVerticalAngle(float angle);
		float verticalAngle() const;

		void setHorizontalAngle(float angle);
		float horizontalAngle() const;
	private:
		uint32 mWidth;
		uint32 mHeight;
		float mVAngle;
		float mHAngle;
	};
}