#include "GridMaterial.h"
#include "geometry/FacePoint.h"

namespace PR
{
	GridMaterial::GridMaterial() :
		Material(), mFirst(nullptr), mSecond(nullptr), mGridCount(10)
	{
	}

	void GridMaterial::setFirstMaterial(Material* mat)
	{
		mFirst = mat;
	}

	Material* GridMaterial::firstMaterial() const
	{
		return mFirst;
	}

	void GridMaterial::setSecondMaterial(Material* mat)
	{
		mSecond = mat;
	}

	Material* GridMaterial::secondMaterial() const
	{
		return mSecond;
	}

	void GridMaterial::setGridCount(int i)
	{
		mGridCount = i;
	}

	int GridMaterial::gridCount() const
	{
		return mGridCount;
	}

	Spectrum GridMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li)
	{
		int u = (int)(PM::pm_GetX(point.uv()) * mGridCount);
		int v = (int)(PM::pm_GetY(point.uv()) * mGridCount);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->apply(point, V, L, Li);
		}
		else if (mSecond)
		{
			return mSecond->apply(point, V, L, Li);
		}
	
		return Spectrum();
	}

	float GridMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		int u = (int)(PM::pm_GetX(point.uv()) * mGridCount);
		int v = (int)(PM::pm_GetY(point.uv()) * mGridCount);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->emitReflectionVector(point, V, dir);
		}
		else if (mSecond)
		{
			return mSecond->emitReflectionVector(point, V, dir);
		}

		return false;
	}

	float GridMaterial::emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		int u = (int)(PM::pm_GetX(point.uv()) * mGridCount);
		int v = (int)(PM::pm_GetY(point.uv()) * mGridCount);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->emitTransmissionVector(point, V, dir);
		}
		else if (mSecond)
		{
			return mSecond->emitTransmissionVector(point, V, dir);
		}

		return false;
	}

	float GridMaterial::roughness() const
	{
		return PM::pm_MaxT(mFirst ? mFirst->roughness() : 0, mSecond ? mSecond->roughness() : 0);
	}
}