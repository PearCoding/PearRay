#include "ConstData1D.h"

namespace PR
{
	ConstData1D::ConstData1D(float value) :
		Data1D(), mValue(value)
	{
	}

	float ConstData1D::value() const
	{
		return mValue;
	}

	void ConstData1D::setValue(float v)
	{
		mValue = v;
	}

	float ConstData1D::getValue(float u)
	{
		return mValue;
	}
}