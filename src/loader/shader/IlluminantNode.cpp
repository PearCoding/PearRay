#include "IlluminantNode.h"

namespace PR {
#include "IlluminantData.inl"

#define _ILLUMINANT_DEC(Prefix, Samples, Start, End)                                                        \
	Prefix##Illuminant::Prefix##Illuminant(float power)                                                     \
		: EquidistantSpectrumViewNode(EquidistantSpectrumView(Prefix##_data, Samples, Start, End)) \
		, mPower(power)                                                                                     \
	{                                                                                                       \
	}                                                                                                       \
	SpectralBlob Prefix##Illuminant::eval(const ShadingContext& ctx) const                                  \
	{                                                                                                       \
		return EquidistantSpectrumViewNode::eval(ctx) * mPower;                                    \
	}                                                                                                       \
	std::string Prefix##Illuminant::dumpInformation() const                                                 \
	{                                                                                                       \
		std::stringstream sstream;                                                                          \
		sstream << "Illuminant " PR_STRINGIFY(Prefix) " (P: " << mPower << ")";                             \
		return sstream.str();                                                                               \
	}

#define _ILLUMINANT_DEC_C(Prefix) _ILLUMINANT_DEC(Prefix, CIE_SampleCount, CIE_WavelengthStart, CIE_WavelengthEnd)
#define _ILLUMINANT_DEC_F(Prefix) _ILLUMINANT_DEC(Prefix, CIE_F_SampleCount, CIE_F_WavelengthStart, CIE_F_WavelengthEnd)

_ILLUMINANT_DEC_C(D65)
_ILLUMINANT_DEC_C(D50)
_ILLUMINANT_DEC_C(D55)
_ILLUMINANT_DEC_C(D75)
_ILLUMINANT_DEC_C(A)
_ILLUMINANT_DEC_C(C)

_ILLUMINANT_DEC_F(F1)
_ILLUMINANT_DEC_F(F2)
_ILLUMINANT_DEC_F(F3)
_ILLUMINANT_DEC_F(F4)
_ILLUMINANT_DEC_F(F5)
_ILLUMINANT_DEC_F(F6)
_ILLUMINANT_DEC_F(F7)
_ILLUMINANT_DEC_F(F8)
_ILLUMINANT_DEC_F(F9)
_ILLUMINANT_DEC_F(F10)
_ILLUMINANT_DEC_F(F11)
_ILLUMINANT_DEC_F(F12)
} // namespace PR
