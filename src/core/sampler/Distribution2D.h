#pragma once

#include "math/Distribution1D.h"
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <vector>
namespace PR {

/* Samples shader in 2D for e.g. good environment mappings 
 * [This is not in base as tbb is a dependency]
*/
class PR_LIB_CORE Distribution2D {
public:
	Distribution2D(size_t width, size_t height);

	inline size_t height() const { return mMarginal.numberOfValues(); }
	inline size_t width() const { return mConditional[0].numberOfValues(); }

	template <typename Func>
	inline void generate(Func func)
	{
		tbb::parallel_for(tbb::blocked_range<size_t>(0, mMarginal.numberOfValues()),
						  [&](const tbb::blocked_range<size_t>& r) {
							  for (size_t y = r.begin(); y != r.end(); ++y)
								  mConditional[y].generate([&](size_t x) { return func(x, y); }, &mConditionalIntegrals[y]);
						  });

		mMarginal.generate([&](size_t y) { return mConditionalIntegrals[y]; });
	}

	Vector2f sampleContinuous(const Vector2f& uv, float& pdf) const;
	float continuousPdf(const Vector2f& uv) const;

private:
	std::vector<Distribution1D> mConditional;
	std::vector<float> mConditionalIntegrals;
	Distribution1D mMarginal;
};
} // namespace PR
