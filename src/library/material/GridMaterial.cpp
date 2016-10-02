#include "GridMaterial.h"
#include "shader/ShaderClosure.h"

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

	Spectrum GridMaterial::eval(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		int u, v;
		auto pointN = applyGrid(point, u, v);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->eval(pointN, L, NdotL);
		}
		else if (mSecond)
		{
			return mSecond->eval(pointN, L, NdotL);
		}
	
		return Spectrum();
	}

	ShaderClosure GridMaterial::applyGrid(const ShaderClosure& point, int& u, int& v) const
	{
		u = (int)(PM::pm_GetX(point.UV) * mGridCount);
		v = (int)(PM::pm_GetY(point.UV) * mGridCount);

		if (mTiledUV)
		{
			ShaderClosure pointN = point;
			pointN.UV = (PM::pm_Set(PM::pm_GetX(point.UV)*mGridCount - u,
				PM::pm_GetY(point.UV)*mGridCount - v
			));
			return pointN;
		}
		else
		{
			return point;
		}
	}

	float GridMaterial::pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		int u, v;
		auto pointN = applyGrid(point, u, v);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->pdf(pointN, L, NdotL);
		}
		else if (mSecond)
		{
			return mSecond->pdf(pointN, L, NdotL);
		}

		return 0;
	}

	PM::vec3 GridMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		int u, v;
		auto pointN = applyGrid(point, u, v);

		if (mFirst && (u % 2) == (v % 2))
		{
			return mFirst->sample(pointN, rnd, pdf);
		}
		else if (mSecond)
		{
			return mSecond->sample(pointN, rnd, pdf);
		}

		pdf = 0;
		return PM::pm_Zero();
	}
}