#pragma once

#include "shader/Socket.h"

namespace PR {
class PR_LIB_UTILS MapShadingSocket : public PR::FloatSpectralShadingSocket {
public:
	MapShadingSocket(const std::shared_ptr<FloatSpectralMapSocket>& map);
	float eval(const ShadingPoint& ctx) const override;
	float relativeLuminance(const ShadingPoint& ctx) const override;
	Vector2i queryRecommendedSize() const override;
	std::string dumpInformation() const override;

private:
	std::shared_ptr<FloatSpectralMapSocket> mMap;
};
} // namespace PR
