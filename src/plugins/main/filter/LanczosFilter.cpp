#include "SceneLoadContext.h"
#include "filter/IFilter.h"
#include "filter/IFilterFactory.h"
#include "filter/IFilterPlugin.h"

#include <vector>

namespace PR {
class LanczosFilter : public IFilter {
public:
	explicit LanczosFilter(int radius = 1)
		: mRadius(radius)
	{
		cache();
	}

	int radius() const override { return mRadius; }
	float evalWeight(float x, float y) const override
	{
		if (mRadius == 0)
			return 1;

		int ix = std::min((int)std::round(std::abs(x)), mRadius + 1);
		int iy = std::min((int)std::round(std::abs(y)), mRadius + 1);

		return mCache.at(iy * (mRadius + 1) + ix);
	}

private:
	inline static float sinc(float x) { return PR_1_PI * std::sin(PR_PI * x) / x; }
	void cache()
	{
		const int halfSize = mRadius + 1;
		mCache.resize(halfSize * (size_t)halfSize);

		auto lanczos = [&](float x) { return x <= PR_EPSILON
												 ? 1.0f
												 : (x <= mRadius
														? sinc(x) * sinc(x / mRadius)
														: 0.0f); };

		float sum1 = 0.0f;
		float sum2 = 0.0f;
		float sum4 = 0.0f;
		for (int y = 0; y < halfSize; ++y) {
			for (int x = 0; x < halfSize; ++x) {
				const float r			 = std::sqrt(x * x + y * y);
				const float val			 = lanczos(r);
				mCache[y * halfSize + x] = val;

				if (y == 0 && x == 0)
					sum1 += val;
				else if (y == 0 || x == 0)
					sum2 += val;
				else
					sum4 += val;
			}
		}

		float norm = 1.0f / (sum1 + 2 * sum2 + 4 * sum4);
		for (int y = 0; y < halfSize; ++y)
			for (int x = 0; x < halfSize; ++x)
				mCache[y * halfSize + x] *= norm;
	}

	int mRadius;
	std::vector<float> mCache;
};

class LanczosFilterFactory : public IFilterFactory {
public:
	explicit LanczosFilterFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<IFilter> createInstance() const override
	{
		int radius = (int)mParams.getInt("radius", 3);
		return std::make_shared<LanczosFilter>(radius);
	}

private:
	ParameterGroup mParams;
};

class LanczosFilterPlugin : public IFilterPlugin {
public:
	std::shared_ptr<IFilterFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<LanczosFilterFactory>(ctx.Parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "lanczos", "sinc", "lancz" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::LanczosFilterPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)