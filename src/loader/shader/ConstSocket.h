#pragma once

#include "shader/Socket.h"
#include "spectral/ParametricBlob.h"

namespace PR {
class PR_LIB_LOADER ConstScalarShadingSocket : public FloatScalarShadingSocket {
public:
	explicit ConstScalarShadingSocket(float f);
	float eval(const ShadingPoint& ctx) const override;
	std::string dumpInformation() const override;

private:
	float mValue;
};

class PR_LIB_LOADER ConstSpectralShadingSocket : public FloatSpectralShadingSocket {
public:
	explicit ConstSpectralShadingSocket(const ParametricBlob& f);
	SpectralBlob eval(const ShadingPoint& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	ParametricBlob mValue;
};

class PR_LIB_LOADER ConstSpectralMapSocket : public FloatSpectralMapSocket {
public:
	explicit ConstSpectralMapSocket(const ParametricBlob& f);
	SpectralBlob eval(const MapSocketCoord& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	ParametricBlob mValue;
};

class PR_LIB_LOADER ConstVectorShadingSocket : public FloatVectorShadingSocket {
public:
	explicit ConstVectorShadingSocket(const Vector3f& f);
	Vector3f eval(const ShadingPoint& ctx) const override;
	std::string dumpInformation() const override;

private:
	Vector3f mValue;
};
} // namespace PR
