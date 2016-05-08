#pragma once

#include "Texture2D.h"

namespace PR
{
	class PR_LIB ConstData2D : public Data2D
	{
	public:
		ConstData2D(float value);

		float value() const;
		void setValue(float value);

	protected:
		float getValue(const PM::vec2& uv) override;

	private:
		float mValue;
	};
}