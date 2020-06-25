#pragma once

#include "EquidistantSpectrumSocket.h"

namespace PR {

#define _ILLUMINANT(Prefix)                                                                \
	class PR_LIB_LOADER Prefix##Illuminant : public EquidistantSpectrumViewNode { \
	public:                                                                                \
		explicit Prefix##Illuminant(float power = 1.0f);                                   \
		SpectralBlob eval(const ShadingContext& ctx) const override;                       \
		std::string dumpInformation() const override;                                      \
                                                                                           \
	private:                                                                               \
		float mPower;                                                                      \
	};

_ILLUMINANT(D65)
_ILLUMINANT(D50)
_ILLUMINANT(D55)
_ILLUMINANT(D75)
_ILLUMINANT(A)
_ILLUMINANT(C)
_ILLUMINANT(F1)
_ILLUMINANT(F2)
_ILLUMINANT(F3)
_ILLUMINANT(F4)
_ILLUMINANT(F5)
_ILLUMINANT(F6)
_ILLUMINANT(F7)
_ILLUMINANT(F8)
_ILLUMINANT(F9)
_ILLUMINANT(F10)
_ILLUMINANT(F11)
_ILLUMINANT(F12)

#undef _ILLUMINANT
} // namespace PR
