#include "RenderRandomMap.h"
#include "RenderContext.h"

namespace PR {
static inline void warmup(Random& rnd, size_t c)
{
	for (size_t i = 0; i < c; ++i)
		(void)rnd.get32();
}

RenderRandomMap::RenderRandomMap(RenderContext* context)
	: mImageSize(context->settings().filmWidth, context->settings().filmHeight)
	, mRandoms(mImageSize.area(), Random(context->settings().seed))
{
	PR_ASSERT(mImageSize.isValid(), "Invalid image size");

	const size_t RngDelta = context->settings().progressive ? 128 : context->settings().maxSampleCount();

	// Warm up
	for (size_t i = 1; i < mRandoms.size(); ++i) {
		mRandoms[i] = mRandoms[i - 1];
		warmup(mRandoms[i], RngDelta);
	}

	// Permute
	for (size_t i = 1; i < mRandoms.size(); ++i)
		std::swap(mRandoms[i], mRandoms[mRandoms[0].get32(1, mRandoms.size())]);
}

} // namespace PR
