#pragma once

#include "entity/Entity.h"

#include <list>

namespace PR
{
	class Camera : public Entity
	{
	public:
		Camera(float width, float height, float vangle, float hangle, const std::string& name, Entity* parent = nullptr);
		virtual ~Camera();

		void setWidth(float w);
		float width() const;

		void setHeight(float h);
		float height() const;

		void setVerticalAngle(float angle);
		float verticalAngle() const;

		void setHorizontalAngle(float angle);
		float horizontalAngle() const;
	private:
		float mWidth;
		float mHeight;
		float mVAngle;
		float mHAngle;
	};
}