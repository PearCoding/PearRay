#pragma once

#include "entity/Entity.h"

#include <list>

namespace PR
{
	class Ray;
	class PR_LIB Camera : public Entity
	{
	public:
		Camera(const std::string& name, Entity* parent = nullptr);
		virtual ~Camera();

		virtual std::string type() const;

		void setWithAngle(float foh, float fov, float lensdist);
		void setWithSize(float width, float height, float lensdist);

		void setWidth(float w);
		float width() const;

		void setHeight(float h);
		float height() const;

		void setLensDistance(float d);
		float lensDistance() const;

		inline PM::mat projectionMatrix() const
		{
			return mMatrix;
		}

		Ray constructRay(float sx, float sy) const;
	private:
		void cache();

		float mWidth;
		float mHeight;
		float mLensDistance;

		PM::mat mMatrix;
	};
}