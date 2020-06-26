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

_OPF1(Abs, abs)
_OPF1(Neg, neg)

_OPF2(Add, add)
_OPF2(Sub, sub)
_OPF2(Mul, mul)
_OPF2(Div, div)
_OPF2(Max, max)
_OPF2(Min, min)

class SpectralMathPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(uint32, const std::string& type_name, const SceneLoadContext& ctx) override
	{
		if (type_name == "sadd") {
			return std::make_shared<AddSpectralMath>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
													 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)));
		} else if (type_name == "ssub") {
			return std::make_shared<SubSpectralMath>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
													 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)));
		} else if (type_name == "smul") {
			return std::make_shared<MulSpectralMath>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
													 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)));
		} else if (type_name == "sdiv") {
			return std::make_shared<DivSpectralMath>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
													 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)));
		} else if (type_name == "sneg") {
			return std::make_shared<NegSpectralMath>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)));
		} else if (type_name == "sabs") {
			return std::make_shared<AbsSpectralMath>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)));
		} else if (type_name == "smax") {
			return std::make_shared<MaxSpectralMath>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
													 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)));
		} else if (type_name == "smin") {
			return std::make_shared<MinSpectralMath>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
													 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)));
		} else {
			PR_ASSERT(false, "SpectralMathNode plugin does not handle all offered types of operations!");
			return nullptr;
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "sadd", "ssub", "smul", "sdiv",
													  "sneg", "sabs",
													  "smax", "smin" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::SpectralMathPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)