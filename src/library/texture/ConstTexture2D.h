#pragma once

#include "Texture2D.h"

namespace PR
{
	class PR_LIB ConstTexture2D : public Texture2D
	{
	public:
		ConstTexture2D(const Spectrum& spec = Spectrum());

		Spectrum value() const;
		void setValue(const Spectrum& spec);

	protected:
		Spectrum getValue(const PM::vec2& uv) override;

	private:
		Spectrum mValue;
	};
}