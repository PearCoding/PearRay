#pragma once

#include "PearMath.h"

namespace PR
{
	class FacePoint
	{
	public:
		FacePoint();
		FacePoint(const PM::vec3& v, const PM::vec3& n, const PM::vec2& u);
		~FacePoint();

		void setVertex(const PM::vec3& v);
		PM::vec3 vertex() const;

		void setNormal(const PM::vec3& v);
		PM::vec3 normal() const;

		void setUV(const PM::vec2& v);
		PM::vec2 uv() const;

	private:
		PM::vec3 mVertex;
		PM::vec3 mNormal;
		PM::vec2 mUV;
	};
}