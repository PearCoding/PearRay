#include "Distribution2D.h"

namespace PR {

Distribution2D::Distribution2D(size_t width, size_t height)
	: mConditional()
	, mConditionalIntegrals(height, 0.0f)
	, mMarginal(height)
{
	for (size_t i = 0; i < height; ++i)
		mConditional.emplace_back(width);
}

Vector2f Distribution2D::sampleContinuous(const Vector2f& uv, float& pdf) const
{
	float pdfs[2];
	size_t moff;
	float d1 = mMarginal.sampleContinuous(uv(1), pdfs[1], &moff);
	float d0 = mConditional[moff].sampleContinuous(uv(0), pdfs[0]);
	pdf		 = pdfs[0] * pdfs[1];
	return Vector2f(d0, d1);
}

float Distribution2D::continuousPdf(const Vector2f& uv) const
{
	size_t moff;
	float pdf1 = mMarginal.continuousPdf(uv(1), &moff);
	float pdf0 = mConditional[moff].continuousPdf(uv(0));
	return pdf0 * pdf1;
}
} // namespace PR
