#pragma once

#include "ShadingPoint.h"

namespace PR {
struct MapSocketCoord {
	Vector2f UV;
	Vector2f dUV = Vector2f(0, 0);
	SpectralBlob WavelengthNM;
	size_t Face = 0;
};

///////////////////
template <typename T>
class ScalarShadingSocket {
public:
	ScalarShadingSocket()		   = default;
	virtual ~ScalarShadingSocket() = default;

	virtual T eval(const ShadingPoint& ctx) const = 0;
	virtual std::string dumpInformation() const	  = 0;
};
using FloatScalarShadingSocket = ScalarShadingSocket<float>;

///////////////////
template <typename M, typename T>
class SpectralSocket {
public:
	SpectralSocket()		  = default;
	virtual ~SpectralSocket() = default;

	virtual SpectralBlobBase<T> eval(const M& ctx) const = 0;
	virtual Vector2i queryRecommendedSize() const		 = 0;
	virtual std::string dumpInformation() const			 = 0;
};

template <typename T>
using SpectralShadingSocket		 = SpectralSocket<ShadingPoint, T>;
using FloatSpectralShadingSocket = SpectralShadingSocket<float>;
template <typename T>
using SpectralMapSocket		 = SpectralSocket<MapSocketCoord, T>;
using FloatSpectralMapSocket = SpectralMapSocket<float>;

///////////////////
template <typename T>
class VectorShadingSocket {
public:
	VectorShadingSocket()		   = default;
	virtual ~VectorShadingSocket() = default;

	virtual Vector3f eval(const ShadingPoint& ctx) const = 0;
	virtual std::string dumpInformation() const				= 0;
};

using FloatVectorShadingSocket = VectorShadingSocket<float>;
} // namespace PR
