#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/INodePlugin.h"

namespace PR {

static inline float evalTriangle(float u, float radius)
{
	return std::max<float>(0.0f, 1.0f - std::abs(u) / radius);
}

class TrianglePeakNode : public FloatSpectralNode {
public:
	explicit TrianglePeakNode(const std::shared_ptr<FloatScalarNode>& peak_nm,
							  const std::shared_ptr<FloatScalarNode>& radius)
		: mPeak(peak_nm)
		, mRadius(radius)
	{
	}

	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		const auto peak	  = mPeak->eval(ctx);
		const auto radius = mRadius->eval(ctx);

		SpectralBlob blob;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			blob[i] = evalTriangle(ctx.WavelengthNM[i] - peak, radius);
		return blob;
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << " TrianglePeak (" << mPeak->dumpInformation() << " , " << mRadius->dumpInformation() << ")";
		return sstream.str();
	}

private:
	const std::shared_ptr<FloatScalarNode> mPeak;
	const std::shared_ptr<FloatScalarNode> mRadius;
};

class TrianglePeakNodeConst : public FloatSpectralNode {
public:
	explicit TrianglePeakNodeConst(float peak_nm, float radius)
		: mPeak(peak_nm)
		, mRadius(radius)
	{
	}

	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		SpectralBlob blob;
		PR_OPT_LOOP
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			blob[i] = evalTriangle(ctx.WavelengthNM[i] - mPeak, mRadius);
		return blob;
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << " TrianglePeak (" << mPeak << " , " << mRadius << ")";
		return sstream.str();
	}

private:
	const float mPeak;
	const float mRadius;
};

class TrianglePeakNodePlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const bool isConst = !ctx.parameters().getParameter(0).isReference() && !ctx.parameters().getParameter(1).isReference();

		if (isConst)
			return std::make_shared<TrianglePeakNodeConst>(ctx.parameters().getParameter(0).getNumber(520),
														   ctx.parameters().getParameter(1).getNumber(2));
		else
			return std::make_shared<TrianglePeakNode>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)),
													  ctx.lookupScalarNode(ctx.parameters().getParameter(1)));
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "triangle_peak", "peak" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::TrianglePeakNodePlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)