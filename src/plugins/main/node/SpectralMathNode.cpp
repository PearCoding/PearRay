#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/INodePlugin.h"

namespace PR {

#define _OPF1(Prefix, F)                                                             \
	class Prefix##SpectralMath : public FloatSpectralNode {                          \
	public:                                                                          \
		explicit Prefix##SpectralMath(const std::shared_ptr<FloatSpectralNode>& op1) \
			: mOp1(op1)                                                              \
		{                                                                            \
		}                                                                            \
		SpectralBlob eval(const ShadingContext& ctx) const override                  \
		{                                                                            \
			return F(mOp1->eval(ctx));                                               \
		}                                                                            \
		Vector2i queryRecommendedSize() const override                               \
		{                                                                            \
			return mOp1->queryRecommendedSize();                                     \
		}                                                                            \
		std::string dumpInformation() const override                                 \
		{                                                                            \
			std::stringstream sstream;                                               \
			sstream << PR_STRINGIFY(F) " (" << mOp1->dumpInformation() << ")";       \
			return sstream.str();                                                    \
		}                                                                            \
                                                                                     \
	private:                                                                         \
		const std::shared_ptr<FloatSpectralNode> mOp1;                               \
	};

#define _OPF2(Prefix, F)                                                                                           \
	class Prefix##SpectralMath : public FloatSpectralNode {                                                        \
	public:                                                                                                        \
		explicit Prefix##SpectralMath(const std::shared_ptr<FloatSpectralNode>& op1,                               \
									  const std::shared_ptr<FloatSpectralNode>& op2)                               \
			: mOp1(op1)                                                                                            \
			, mOp2(op2)                                                                                            \
		{                                                                                                          \
		}                                                                                                          \
		SpectralBlob eval(const ShadingContext& ctx) const override                                                \
		{                                                                                                          \
			return F(mOp1->eval(ctx), mOp2->eval(ctx));                                                            \
		}                                                                                                          \
		Vector2i queryRecommendedSize() const override                                                             \
		{                                                                                                          \
			return mOp1->queryRecommendedSize().cwiseMax(mOp2->queryRecommendedSize());                            \
		}                                                                                                          \
		std::string dumpInformation() const override                                                               \
		{                                                                                                          \
			std::stringstream sstream;                                                                             \
			sstream << PR_STRINGIFY(F) " (" << mOp1->dumpInformation() << " , " << mOp2->dumpInformation() << ")"; \
			return sstream.str();                                                                                  \
		}                                                                                                          \
                                                                                                                   \
	private:                                                                                                       \
		const std::shared_ptr<FloatSpectralNode> mOp1;                                                             \
		const std::shared_ptr<FloatSpectralNode> mOp2;                                                             \
	};

inline static SpectralBlob abs(const SpectralBlob& v)
{
	return v.cwiseAbs();
}
inline static SpectralBlob neg(const SpectralBlob& v) { return -v; }

inline static SpectralBlob add(const SpectralBlob& a, const SpectralBlob& b) { return a + b; }
inline static SpectralBlob sub(const SpectralBlob& a, const SpectralBlob& b) { return a - b; }
inline static SpectralBlob mul(const SpectralBlob& a, const SpectralBlob& b) { return a * b; }
inline static SpectralBlob div(const SpectralBlob& a, const SpectralBlob& b) { return a / b; }
inline static SpectralBlob min(const SpectralBlob& a, const SpectralBlob& b) { return a.cwiseMin(b); }
inline static SpectralBlob max(const SpectralBlob& a, const SpectralBlob& b) { return a.cwiseMax(b); }

// Most of this operations require an upper boundary... we assume 1, which is not sufficient all the time
// Based on https://docs.gimp.org/en/gimp-concepts-layer-modes.html
inline static SpectralBlob burn(const SpectralBlob& a, const SpectralBlob& b) { return 1 - (1 - a) / (b + 1); }
inline static SpectralBlob screen(const SpectralBlob& a, const SpectralBlob& b) { return 1 - (1 - a) * (1 - b); }
inline static SpectralBlob dodge(const SpectralBlob& a, const SpectralBlob& b) { return a / ((1 - b) + 1); }
inline static SpectralBlob overlay(const SpectralBlob& a, const SpectralBlob& b) { return a * (a + 2 * b * (1 - a)); }
inline static SpectralBlob softlight(const SpectralBlob& a, const SpectralBlob& b)
{
	SpectralBlob r = screen(a, b);
	return ((1 - a) * b + r) * a;
}
inline static SpectralBlob hardlight(const SpectralBlob& a, const SpectralBlob& b)
{
	SpectralBlob k1 = 1 - (1 - 2 * (b - 0.5f)) * (1 - a);
	SpectralBlob k2 = 2 * b * a;
	return (b <= 0.5f).select(k2, k1);
}

_OPF1(Abs, abs)
_OPF1(Neg, neg)

_OPF2(Add, add)
_OPF2(Sub, sub)
_OPF2(Mul, mul)
_OPF2(Div, div)
_OPF2(Max, max)
_OPF2(Min, min)

_OPF2(Burn, burn)
_OPF2(Screen, screen)
_OPF2(Dodge, dodge)
_OPF2(Overlay, overlay)
_OPF2(Softlight, softlight)
_OPF2(Hardlight, hardlight)

class ConstBlendSpectralMath : public FloatSpectralNode {
public:
	explicit ConstBlendSpectralMath(float factor,
									const std::shared_ptr<FloatSpectralNode>& op1,
									const std::shared_ptr<FloatSpectralNode>& op2)
		: mFactor(factor)
		, mOp1(op1)
		, mOp2(op2)
	{
	}
	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		return mOp1->eval(ctx) * (1 - mFactor) + mOp2->eval(ctx) * mFactor;
	}

	Vector2i queryRecommendedSize() const override
	{
		return mOp1->queryRecommendedSize().cwiseMax(mOp2->queryRecommendedSize());
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Blend (" << mFactor << ", " << mOp1->dumpInformation() << " , " << mOp2->dumpInformation() << ")";
		return sstream.str();
	}

private:
	const float mFactor;
	const std::shared_ptr<FloatSpectralNode> mOp1;
	const std::shared_ptr<FloatSpectralNode> mOp2;
};

class DynamicBlendSpectralMath : public FloatSpectralNode {
public:
	explicit DynamicBlendSpectralMath(const std::shared_ptr<FloatScalarNode>& factor,
									  const std::shared_ptr<FloatSpectralNode>& op1,
									  const std::shared_ptr<FloatSpectralNode>& op2)
		: mFactor(factor)
		, mOp1(op1)
		, mOp2(op2)
	{
	}
	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		const float f = mFactor->eval(ctx);
		return mOp1->eval(ctx) * (1 - f) + mOp2->eval(ctx) * f;
	}

	Vector2i queryRecommendedSize() const override
	{
		return mOp1->queryRecommendedSize().cwiseMax(mOp2->queryRecommendedSize());
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Blend (" << mFactor->dumpInformation() << ", " << mOp1->dumpInformation() << " , " << mOp2->dumpInformation() << ")";
		return sstream.str();
	}

private:
	const std::shared_ptr<FloatScalarNode> mFactor;
	const std::shared_ptr<FloatSpectralNode> mOp1;
	const std::shared_ptr<FloatSpectralNode> mOp2;
};

class SpectralMathPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(uint32, const std::string& type_name, const SceneLoadContext& ctx) override
	{
		if (type_name == "sadd") {
			return std::make_shared<AddSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
													 ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "ssub") {
			return std::make_shared<SubSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
													 ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "smul") {
			return std::make_shared<MulSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
													 ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "sdiv") {
			return std::make_shared<DivSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
													 ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "sneg") {
			return std::make_shared<NegSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "sabs") {
			return std::make_shared<AbsSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "smax") {
			return std::make_shared<MaxSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
													 ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "smin") {
			return std::make_shared<MinSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
													 ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "sburn") {
			return std::make_shared<BurnSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
													  ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "sscreen") {
			return std::make_shared<ScreenSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
														ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "sdodge") {
			return std::make_shared<DodgeSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
													   ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "soverlay") {
			return std::make_shared<OverlaySpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
														 ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "ssoftlight") {
			return std::make_shared<SoftlightSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
														   ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "shardlight") {
			return std::make_shared<HardlightSpectralMath>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
														   ctx.lookupSpectralNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "sblend") {
			const auto fac = ctx.parameters().getParameter(0);
			if (fac.isReference())
				return std::make_shared<DynamicBlendSpectralMath>(ctx.lookupScalarNode(fac),
																  ctx.lookupSpectralNode(ctx.parameters().getParameter(1)),
																  ctx.lookupSpectralNode(ctx.parameters().getParameter(2)));
			else
				return std::make_shared<ConstBlendSpectralMath>(fac.getNumber(0.5f),
																ctx.lookupSpectralNode(ctx.parameters().getParameter(1)),
																ctx.lookupSpectralNode(ctx.parameters().getParameter(2)));
		} else {
			PR_ASSERT(false, "SpectralMathNode plugin does not handle all offered types of operations!");
			return nullptr;
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "sadd", "ssub", "smul", "sdiv",
													  "sneg", "sabs",
													  "smax", "smin",
													  "sburn", "sscreen", "sdodge", "soverlay", "ssoftlight", "shardlight",
													  "sblend" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SpectralMathPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)