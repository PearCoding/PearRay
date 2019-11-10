#include "MapShadingSocket.h"

namespace PR {
MapShadingSocket::MapShadingSocket(const std::shared_ptr<FloatSpectralMapSocket>& map)
	: FloatSpectralShadingSocket()
	, mMap(map)
{
	PR_ASSERT(map, "Given map socket has to be valid");
}

float MapShadingSocket::eval(const ShadingPoint& ctx) const
{
	MapSocketCoord coord;
	coord.UV   = Vector2f(ctx.Geometry.UVW[0], ctx.Geometry.UVW[1]);
	coord.Face = ctx.PrimID;

	return mMap->eval(coord);
}

float MapShadingSocket::relativeLuminance(const ShadingPoint& ctx) const
{
	MapSocketCoord coord;
	coord.UV   = Vector2f(ctx.Geometry.UVW[0], ctx.Geometry.UVW[1]);
	coord.Face = ctx.PrimID;

	return mMap->eval(coord);
}

Vector2i MapShadingSocket::queryRecommendedSize() const
{
	return mMap->queryRecommendedSize();
}

std::string MapShadingSocket::dumpInformation() const
{
	return mMap->dumpInformation();
}
} // namespace PR
