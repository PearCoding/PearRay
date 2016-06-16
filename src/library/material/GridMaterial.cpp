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

	Spectrum GridMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		int u, v;
		auto pointN = applyGrid(point, u, v);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->apply(pointN, V, L);
		}
		else if (mSecond)
		{
			return mSecond->apply(pointN, V, L);
		}
	
		return Spectrum();
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

	float GridMaterial::pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		int u, v;
		auto pointN = applyGrid(point, u, v);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->pdf(pointN, V, L);
		}
		else if (mSecond)
		{
			return mSecond->pdf(pointN, V, L);
		}

		return 0;
	}

	PM::vec3 GridMaterial::sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf)
	{
		int u, v;
		auto pointN = applyGrid(point, u, v);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->sample(pointN, rnd, V, pdf);
		}
		else if (mSecond)
		{
			return mSecond->sample(pointN, rnd, V, pdf);
		}

		pdf = 0;
		return PM::pm_Zero();
	}
}