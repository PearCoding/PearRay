#pragma once

#include "shader/Socket.h"
#include "spectral/Spectrum.h"

namespace PR {
class PR_LIB_UTILS ConstScalarShadingSocket : public FloatScalarShadingSocket {
public:
	explicit ConstScalarShadingSocket(float f);
	float eval(const ShadingPoint& ctx) const override;
	std::string dumpInformation() const override;

private:
	float mValue;
};

class PR_LIB_UTILS ConstSpectralShadingSocket : public FloatSpectralShadingSocket {
public:
	explicit ConstSpectralShadingSocket(const Spectrum& f);
	float eval(const ShadingPoint& ctx) const override;
	float relativeLuminance(const ShadingPoint& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	Spectrum mValue;
};

class PR_LIB_UTILS ConstSpectralMapSocket : public FloatSpectralMapSocket {
public:
	explicit ConstSpectralMapSocket(const Spectrum& f);
	float eval(const MapSocketCoord& ctx) const override;
	float relativeLuminance(const MapSocketCoord& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	Spectrum mValue;
};

class PR_LIB_UTILS ConstVectorShadingSocket : public FloatVectorShadingSocket {
public:
	explicit ConstVectorShadingSocket(const Vector3f& f);
	float eval(uint32 channel, const ShadingPoint& ctx) const override;
	std::string dumpInformation() const override;

private:
	Vector3f mValue;
};
} // namespace PR
