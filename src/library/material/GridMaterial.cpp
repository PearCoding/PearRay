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

	void GridMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li,
		Spectrum& diff, Spectrum& spec)
	{
		int u = PM::pm_GetX(point.uv()) * mGridCount;
		int v = PM::pm_GetY(point.uv()) * mGridCount;

		if (mFirst && (u % 2) == (v % 2))
		{
			mFirst->apply(point, V, L, Li, diff, spec);
		}
		else if (mSecond)
		{
			mSecond->apply(point, V, L, Li, diff, spec);
		}
	}

	float GridMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		int u = PM::pm_GetX(point.uv()) * mGridCount;
		int v = PM::pm_GetY(point.uv()) * mGridCount;

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
		int u = PM::pm_GetX(point.uv()) * mGridCount;
		int v = PM::pm_GetY(point.uv()) * mGridCount;

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