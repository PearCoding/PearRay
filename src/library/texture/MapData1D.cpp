#include "MapData1D.h"

namespace PR
{
	MapData1D::MapData1D(float* values, uint32 width) :
		Data1D(), mValues(values), mWidth(width)
	{
		PR_ASSERT(values);
		PR_ASSERT(width > 0);
	}

	MapData1D::~MapData1D()
	{
		delete[] mValues;
	}

	uint32 MapData1D::width() const
	{
		return mWidth;
	}

	float* MapData1D::entries() const
	{
		return mValues;
	}

	void MapData1D::setEntry(uint32 px, float value)
	{
		mValues[px] = value;
	}

	float MapData1D::entry(uint32 px) const
	{
		return mValues[px];
	}

	float MapData1D::getValue(float u)
	{
		// TODO: Use interpolation!
		uint32 px = uint32(u * mWidth);
		return entry(px);
	}
}