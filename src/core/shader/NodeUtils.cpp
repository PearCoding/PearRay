#include "NodeUtils.h"
#include "ShadingContext.h"
#include "math/Bits.h"

namespace PR {

constexpr uint64 SX = 32;
constexpr uint64 SY = 32;
constexpr uint64 SC = SX * SY;
template <typename F>
inline float compute(const SpectralBlob& wvls, const F& func)
{
	ShadingContext sc;
	sc.WavelengthNM = wvls;

	float sum = 0;
	for (uint64 i = 0; i < SC; ++i) {
		uint32 x, y;
		morton_2_xy(i, x, y);

		PR_ASSERT(x < SX && y < SY, "Invalid morton code");
		sc.UV = Vector2f(x / float(SX), y / float(SY));

		sum += func(sc);
	}

	return sum / SC;
}

float NodeUtils::average(const SpectralBlob& wvls, FloatScalarNode* node)
{
	return compute(wvls, [=](const ShadingContext& sc) { return node->eval(sc); });
}

float NodeUtils::average(const SpectralBlob& wvls, FloatSpectralNode* node)
{
	return compute(wvls, [=](const ShadingContext& sc) { return node->eval(sc).mean(); });
}

} // namespace PR
