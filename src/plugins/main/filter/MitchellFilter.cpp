#include "SceneLoadContext.h"
#include "filter/IFilter.h"
#include "filter/IFilterFactory.h"
#include "filter/IFilterPlugin.h"

#include <vector>

namespace PR {
class MitchellFilter : public IFilter {
public:
	explicit MitchellFilter(int radius = 1)
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
	// [-2,2]
	static inline float mitchell(float x, float B, float C)
	{
		x = std::abs(x);
		if (x < 1)
			return ((12 - 9 * B - 6 * C) * x * x * x + (-18 + 12 * B + 6 * C) * x * x + (6 - 2 * B)) / 6;
		else if (x < 2)
			return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x + (-12 * B - 48 * C) * x + (8 * B + 24 * C)) / 6;
		else
			return 0;
	}
	void cache()
	{
		if (mRadius == 0)
			return;

		const int halfSize = mRadius + 1;
		mCache.resize(halfSize * (size_t)halfSize);

		auto filter = [=](float x) { return mitchell(2 * x / mRadius, 1 / 3.0f, 1 / 3.0f); };

		float sum1 = 0.0f;
		float sum2 = 0.0f;
		float sum4 = 0.0f;
		for (int y = 0; y < halfSize; ++y) {
			for (int x = 0; x < halfSize; ++x) {
				const float r			 = std::sqrt(x * x + y * y);
				const float val			 = filter(r);
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

class MitchellFilterFactory : public IFilterFactory {
public:
	explicit MitchellFilterFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<IFilter> createInstance() const override
	{
		int radius = (int)mParams.getInt("radius", 3);
		return std::make_shared<MitchellFilter>(radius);
	}

private:
	ParameterGroup mParams;
};

class MitchellFilterPlugin : public IFilterPlugin {
public:
	std::shared_ptr<IFilterFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<MitchellFilterFactory>(ctx.Parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "mitchell", "default" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::MitchellFilterPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)