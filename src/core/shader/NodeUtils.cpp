#include "NodeUtils.h"
#include "ShadingContext.h"
#include "math/Bits.h"

namespace PR {

constexpr uint64 SX = 32;
constexpr uint64 SY = 32;
constexpr uint64 SC = SX * SY;

static inline Vector2f generateUV(uint64 i)
{
	uint32 x, y;
	morton_2_xy(i, x, y);

	PR_ASSERT(x < SX && y < SY, "Invalid morton code");
	return Vector2f(x / float(SX), y / float(SY));
}

template <typename Ret, typename F>
inline Ret compute(const SpectralBlob& wvls, const F& func)
{
	ShadingContext sc;
	sc.WavelengthNM = wvls;
	sc.UV			= generateUV(0);

	Ret sum = func(sc);

	for (uint64 i = 1; i < SC; ++i) {
		sc.UV = generateUV(i);
		sum += func(sc);
	}

	return sum / SC;
}

float NodeUtils::average(const SpectralBlob& wvls, const FloatScalarNode* node)
{
	return compute<float>(wvls, [=](const ShadingContext& sc) -> float { return node->eval(sc); });
}

SpectralBlob NodeUtils::average(const SpectralBlob& wvls, const FloatSpectralNode* node)
{
	return compute<SpectralBlob>(wvls, [=](const ShadingContext& sc) -> SpectralBlob { return node->eval(sc); });
}

} // namespace PR
