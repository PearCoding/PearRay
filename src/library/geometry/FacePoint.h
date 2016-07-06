#pragma once

#include "Config.h"
#include "PearMath.h"

namespace PR
{
	class Material;
	class PR_LIB FacePoint
	{
	public:
		FacePoint();
		FacePoint(const PM::vec3& v, const PM::vec3& n, const PM::vec3& t, const PM::vec3& b, const PM::vec2& u);
		~FacePoint();

		void setVertex(const PM::vec3& v);
		PM::vec3 vertex() const;

		void setNormal(const PM::vec3& v);
		PM::vec3 normal() const;

		void setTangent(const PM::vec3& v);
		PM::vec3 tangent() const;

		void setBinormal(const PM::vec3& v);
		PM::vec3 binormal() const;

		void calculateTangentFrame();

		void setUV(const PM::vec2& v);
		PM::vec2 uv() const;

		void setInside(bool b);
		bool isInside() const;

		void setMaterial(Material* m);
		Material* material() const;

	private:
		alignas(16) PM::vec3 mVertex;
		alignas(16) PM::vec3 mNormal;
		alignas(16) PM::vec3 mTangent;
		alignas(16) PM::vec3 mBinormal;
		alignas(16) PM::vec2 mUV;
		bool mInside;

		Material* mMaterial;
	};
}