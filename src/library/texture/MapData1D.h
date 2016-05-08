#pragma once

#include "Texture1D.h"

namespace PR
{
	class PR_LIB MapData1D : public Data1D
	{
	public:
		MapData1D(float* entries, uint32 width);
		~MapData1D();

		uint32 width() const;

		float* entries() const;

		void setEntry(uint32 px, float value);
		float entry(uint32 px) const;

	protected:
		float getValue(float u) override;

	private:
		float* mValues;
		uint32 mWidth;
	};
}