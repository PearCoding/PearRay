#include "FacePoint.h"

namespace PR
{
	FacePoint::FacePoint() :
		mVertex(PM::pm_Set(0,0,0,1)), mNormal(PM::pm_Set(0,0,0)), mUV(PM::pm_Set(0,0)), mInside(false)
	{
	}

	FacePoint::FacePoint(const PM::vec3& v, const PM::vec3& n, const PM::vec2& u) :
		mVertex(v), mNormal(n), mUV(u), mInside(false)
	{
	}

	FacePoint::~FacePoint()
	{
	}

	void FacePoint::setVertex(const PM::vec3& v)
	{
		mVertex = v;
	}

	PM::vec3 FacePoint::vertex() const
	{
		return mVertex;
	}

	void FacePoint::setNormal(const PM::vec3& n)
	{
		mNormal = n;
	}

	PM::vec3 FacePoint::normal() const
	{
		return mNormal;
	}

	void FacePoint::setUV(const PM::vec2& u)
	{
		mUV = u;
	}

	PM::vec2 FacePoint::uv() const
	{
		return mUV;
	}

	void FacePoint::setInside(bool b)
	{
		mInside = b;
	}

	bool FacePoint::isInside() const
	{
		return mInside;
	}
}