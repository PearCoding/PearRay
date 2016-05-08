#pragma once

#include "Texture1D.h"

namespace PR
{
	class PR_LIB ConstData1D : public Data1D
	{
	public:
		ConstData1D(float val);

		float value() const;
		void setValue(float val);

	protected:
		float getValue(float u) override;

	private:
		float mValue;
	};
}