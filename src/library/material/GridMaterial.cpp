#include "GridMaterial.h"
#include "geometry/FacePoint.h"

namespace PR
{
	GridMaterial::GridMaterial() :
		Material(), mFirst(nullptr), mSecond(nullptr), mGridCount(10), mCameraVisible(true)
	{
	}
	
	bool GridMaterial::isLight() const
	{
		return (mFirst ? mFirst->isLight() : false) || (mSecond ? mSecond->isLight() : false);
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

	void GridMaterial::enableCameraVisibility(bool b)
	{
		mCameraVisible = b;
	}

	bool GridMaterial::isCameraVisible() const
	{
		return mCameraVisible;
	}

	void GridMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
		int u = PM::pm_GetX(point.uv()) * mGridCount;
		int v = PM::pm_GetY(point.uv()) * mGridCount;

		if (mFirst && (u % 2) == (v % 2))
		{
			mFirst->apply(in, entity, point, renderer);
		}
		else if (mSecond)
		{
			mSecond->apply(in, entity, point, renderer);
		}
	}
}