#include "MapData2D.h"

namespace PR
{
	MapData2D::MapData2D(float* values, uint32 width, uint32 height) :
		Data2D(), mValues(values), mWidth(width), mHeight(height)
	{
		PR_ASSERT(values);
		PR_ASSERT(width > 0);
		PR_ASSERT(height > 0);
	}

	MapData2D::~MapData2D()
	{
		delete[] mValues;
	}

	uint32 MapData2D::width() const
	{
		return mWidth;
	}

	uint32 MapData2D::height() const
	{
		return mHeight;
	}

	float* MapData2D::entries() const
	{
		return mValues;
	}

	void MapData2D::setEntry(uint32 px, uint32 py, float value)
	{
		mValues[py * mWidth + px] = value;
	}

	float MapData2D::entry(uint32 px, uint32 py) const
	{
		return mValues[py * mWidth + px];
	}

	float MapData2D::getValue(const PM::vec2& uv)
	{
		// TODO: Use interpolation!
		uint32 px = uint32(PM::pm_GetX(uv) * mWidth);
		uint32 py = uint32(PM::pm_GetY(uv) * mHeight);
		return entry(px, py);
	}
}