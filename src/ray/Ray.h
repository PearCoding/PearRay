#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class Ray
	{
	public:
		Ray(const PM::vec3& pos, const PM::vec3& dir);
		virtual ~Ray();

		void setStartPosition(const PM::vec3& p);
		PM::vec3 startPosition() const;

		void setDirection(const PM::vec3& p);
		PM::vec3 direction() const;

		void setIntensity(float f);
		float intensity() const;

	private:
		PM::vec3 mStartPosition;
		PM::vec3 mDirection;

		float mIntensity;
	};
}