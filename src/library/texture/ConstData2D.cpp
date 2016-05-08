#include "ConstData2D.h"

namespace PR
{
	ConstData2D::ConstData2D(float value) :
		Data2D(), mValue(value)
	{
	}

	float ConstData2D::value() const
	{
		return mValue;
	}

	void ConstData2D::setValue(float value)
	{
		mValue = value;
	}

	float ConstData2D::getValue(const PM::vec2& uv)
	{
		return mValue;
	}
}