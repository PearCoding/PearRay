#pragma once

#include "Camera.h"

namespace PR
{
	class PR_LIB OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(const std::string& name, Entity* parent = nullptr);
		virtual ~OrthographicCamera();

		virtual std::string type() const;

		void setWithAngle(float foh, float fov, float lensdist);
		void setWithSize(float width, float height, float lensdist);

		void setWidth(float w);
		float width() const;

		void setHeight(float h);
		float height() const;

		void setLensDistance(float d);
		float lensDistance() const;

		Ray constructRay(float nx, float ny) const;
	private:
		float mWidth;
		float mHeight;
		float mLensDistance;
	};
}