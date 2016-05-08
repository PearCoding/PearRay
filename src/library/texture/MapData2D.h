#pragma once

#include "Texture2D.h"

namespace PR
{
	class PR_LIB MapData2D : public Data2D
	{
	public:
		MapData2D(float* values, uint32 width, uint32 height);
		~MapData2D();

		uint32 width() const;
		uint32 height() const;

		float* entries() const;

		void setEntry(uint32 px, uint32 py, float value);
		float entry(uint32 px, uint32 py) const;

	protected:
		float getValue(const PM::vec2& uv) override;

	private:
		float* mValues;
		uint32 mWidth;
		uint32 mHeight;
	};
}