#pragma once

#include "Texture1D.h"

namespace PR
{
	class PR_LIB ConstTexture1D : public Texture1D
	{
	public:
		ConstTexture1D(const Spectrum& spec = Spectrum());

		Spectrum value() const;
		void setValue(const Spectrum& spec);

	protected:
		Spectrum getValue(float u) override;

	private:
		Spectrum mValue;
	};
}