#include "SceneLoadContext.h"
#include "filter/IFilter.h"
#include "filter/IFilterFactory.h"
#include "filter/IFilterPlugin.h"

#include <vector>

namespace PR {
class GaussianFilter : public IFilter {
public:
	explicit GaussianFilter(int radius = 1)
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
	void cache()
	{
		if (mRadius == 0)
			return;

		const int halfSize = mRadius + 1;
		mCache.resize(halfSize * (size_t)halfSize);

		auto gauss = [](float x, float alpha) {
			return x <= 1.0f ? std::exp(-alpha * x * x) : 0.0f;
		};

		const float dev2  = 0.2f;
		const float alpha = 1 / (2 * dev2);

		float sum1 = 0.0f;
		float sum2 = 0.0f;
		float sum4 = 0.0f;
		for (int y = 0; y < halfSize; ++y) {
			for (int x = 0; x < halfSize; ++x) {
				const float r			 = std::sqrt(x * x + y * y) / (float)mRadius;
				const float val			 = gauss(r, alpha);
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

class GaussianFilterFactory : public IFilterFactory {
public:
	explicit GaussianFilterFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<IFilter> createInstance() const override
	{
		int radius = (int)mParams.getInt("radius", 3);
		return std::make_shared<GaussianFilter>(radius);
	}

private:
	ParameterGroup mParams;
};

class GaussianFilterPlugin : public IFilterPlugin {
public:
	std::shared_ptr<IFilterFactory> create(const std::string&, const SceneLoadContext& ctx) override
	{
		return std::make_shared<GaussianFilterFactory>(ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "gaussian", "gauss" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Gaussian Filter", "Simple filter based on the gaussian funtion")
			.Identifiers(getNames())
			.Inputs()
			.UInt("radius", "Radius of filter", 3)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::GaussianFilterPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)