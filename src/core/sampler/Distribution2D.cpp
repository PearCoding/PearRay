#include "Distribution2D.h"

namespace PR {

Distribution2D::Distribution2D(size_t width, size_t height)
	: mConditional()
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

/* Apply the paper
 * Ondřej Karlík, Martin Šik, Petr Vévoda, Tomáš Skřivan, and Jaroslav Křivánek.
 * MIS Compensation: Optimizing Sampling Techniques in Multiple Importance Sampling. 
 * ACM Transactions on Graphics (Proceedings of SIGGRAPH Asia 2019), 38(6), 2019.
 */
void Distribution2D::applyCompensation()
{
	// Get average PDF for all row conditionals
	std::vector<float> avgs;
	avgs.resize(height(), 0.0f);

	const size_t w = width();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, height()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t y = r.begin(); y != r.end(); ++y) {
							  float& avg	  = avgs[y];
							  const auto& cdf = mConditional[y];
							  for (size_t x = 0; x < w; ++x)
								  avg += cdf.discretePdf(x);
							  avg /= w;
						  }
					  });

	// Extract single average from row averages
	float single_avg = 0;
	for (float f : avgs)
		single_avg += f;
	single_avg /= height();

	// Reduce by the average
	std::vector<float> integrals = std::move(avgs); // Rename for memory reasons

	tbb::parallel_for(tbb::blocked_range<size_t>(0, height()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t y = r.begin(); y != r.end(); ++y)
							  mConditional[y].reducePDFBy(single_avg, &integrals[y]);
					  });

	mMarginal.generate([&](size_t y) { return integrals[y]; });
}
} // namespace PR
