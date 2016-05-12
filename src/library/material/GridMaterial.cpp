#include "GridMaterial.h"
#include "geometry/FacePoint.h"

namespace PR
{
	GridMaterial::GridMaterial() :
		Material(), mFirst(nullptr), mSecond(nullptr), mGridCount(10), mTiledUV(true)
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

	void GridMaterial::setTileUV(bool b)
	{
		mTiledUV = b;
	}

	bool GridMaterial::tileUV() const
	{
		return mTiledUV;
	}

	Spectrum GridMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li)
	{
		int u, v;
		auto pointN = applyGrid(point, u, v);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->apply(pointN, V, L, Li);
		}
		else if (mSecond)
		{
			return mSecond->apply(pointN, V, L, Li);
		}
	
		return Spectrum();
	}

	float GridMaterial::emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		int u, v;
		auto pointN = applyGrid(point, u, v);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->emitReflectionVector(pointN, V, dir);
		}
		else if (mSecond)
		{
			return mSecond->emitReflectionVector(pointN, V, dir);
		}

		return false;
	}

	float GridMaterial::emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir)
	{
		int u, v;
		auto pointN = applyGrid(point, u, v);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->emitTransmissionVector(pointN, V, dir);
		}
		else if (mSecond)
		{
			return mSecond->emitTransmissionVector(pointN, V, dir);
		}

		return false;
	}

	float GridMaterial::roughness(const FacePoint& point) const
	{
		return PM::pm_MaxT(mFirst ? mFirst->roughness(point) : 0, mSecond ? mSecond->roughness(point) : 0);
	}

	FacePoint GridMaterial::applyGrid(const FacePoint& point, int& u, int& v) const
	{
		u = (int)(PM::pm_GetX(point.uv()) * mGridCount);
		v = (int)(PM::pm_GetY(point.uv()) * mGridCount);

		if (mTiledUV)
		{
			FacePoint pointN = point;
			pointN.setUV(PM::pm_Set(PM::pm_GetX(point.uv())*mGridCount - u,
				PM::pm_GetY(point.uv())*mGridCount - v
			));
			return pointN;
		}
		else
		{
			return point;
		}
	}
}