#pragma once

#include "entity/Entity.h"

#include <list>

namespace PR
{
	class Ray;
	class PR_LIB Camera : public Entity
	{
	public:
		Camera(float width, float height, float lensDistance, const std::string& name, Entity* parent = nullptr);
		virtual ~Camera();

		void setWidth(float w);
		float width() const;

		void setHeight(float h);
		float height() const;

		void setLensDistance(float d);
		float lensDistance() const;

		Ray constructRay(float sx, float sy) const;
	private:
		float mWidth;
		float mHeight;
		float mLensDistance;
	};
}