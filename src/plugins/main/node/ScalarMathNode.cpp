#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/INodePlugin.h"

namespace PR {
#define _OP1(Prefix, Op)                                                        \
	class Prefix##Math : public FloatScalarNode {                               \
	public:                                                                     \
		explicit Prefix##Math(const std::shared_ptr<FloatScalarNode>& op1)      \
			: mOp1(op1)                                                         \
		{                                                                       \
		}                                                                       \
		float eval(const ShadingContext& ctx) const override                    \
		{                                                                       \
			return Op mOp1->eval(ctx);                                          \
		}                                                                       \
		std::string dumpInformation() const override                            \
		{                                                                       \
			std::stringstream sstream;                                          \
			sstream << PR_STRINGIFY(Op) " (" << mOp1->dumpInformation() << ")"; \
			return sstream.str();                                               \
		}                                                                       \
                                                                                \
	private:                                                                    \
		const std::shared_ptr<FloatScalarNode> mOp1;                            \
	};

#define _OPF1(Prefix, F)                                                       \
	class Prefix##Math : public FloatScalarNode {                              \
	public:                                                                    \
		explicit Prefix##Math(const std::shared_ptr<FloatScalarNode>& op1)     \
			: mOp1(op1)                                                        \
		{                                                                      \
		}                                                                      \
		float eval(const ShadingContext& ctx) const override                   \
		{                                                                      \
			return F(mOp1->eval(ctx));                                         \
		}                                                                      \
		std::string dumpInformation() const override                           \
		{                                                                      \
			std::stringstream sstream;                                         \
			sstream << PR_STRINGIFY(F) " (" << mOp1->dumpInformation() << ")"; \
			return sstream.str();                                              \
		}                                                                      \
                                                                               \
	private:                                                                   \
		const std::shared_ptr<FloatScalarNode> mOp1;                           \
	};

#define _OP2(Prefix, Op)                                                                                               \
	class Prefix##Math : public FloatScalarNode {                                                                      \
	public:                                                                                                            \
		explicit Prefix##Math(const std::shared_ptr<FloatScalarNode>& op1,                                             \
							  const std::shared_ptr<FloatScalarNode>& op2)                                             \
			: mOp1(op1)                                                                                                \
			, mOp2(op2)                                                                                                \
		{                                                                                                              \
		}                                                                                                              \
		float eval(const ShadingContext& ctx) const override                                                           \
		{                                                                                                              \
			return mOp1->eval(ctx) Op mOp2->eval(ctx);                                                                 \
		}                                                                                                              \
		std::string dumpInformation() const override                                                                   \
		{                                                                                                              \
			std::stringstream sstream;                                                                                 \
			sstream << "(" << mOp1->dumpInformation() << ") " PR_STRINGIFY(Op) " (" << mOp2->dumpInformation() << ")"; \
			return sstream.str();                                                                                      \
		}                                                                                                              \
                                                                                                                       \
	private:                                                                                                           \
		const std::shared_ptr<FloatScalarNode> mOp1;                                                                   \
		const std::shared_ptr<FloatScalarNode> mOp2;                                                                   \
	};

#define _OPF2(Prefix, F)                                                                                           \
	class Prefix##Math : public FloatScalarNode {                                                                  \
	public:                                                                                                        \
		explicit Prefix##Math(const std::shared_ptr<FloatScalarNode>& op1,                                         \
							  const std::shared_ptr<FloatScalarNode>& op2)                                         \
			: mOp1(op1)                                                                                            \
			, mOp2(op2)                                                                                            \
		{                                                                                                          \
		}                                                                                                          \
		float eval(const ShadingContext& ctx) const override                                                       \
		{                                                                                                          \
			return F(mOp1->eval(ctx), mOp2->eval(ctx));                                                            \
		}                                                                                                          \
		std::string dumpInformation() const override                                                               \
		{                                                                                                          \
			std::stringstream sstream;                                                                             \
			sstream << PR_STRINGIFY(F) " (" << mOp1->dumpInformation() << " , " << mOp2->dumpInformation() << ")"; \
			return sstream.str();                                                                                  \
		}                                                                                                          \
                                                                                                                   \
	private:                                                                                                       \
		const std::shared_ptr<FloatScalarNode> mOp1;                                                               \
		const std::shared_ptr<FloatScalarNode> mOp2;                                                               \
	};

#define ADD_OP +
#define SUB_OP -
#define MUL_OP *
#define DIV_OP /

_OP2(Add, ADD_OP)
_OP2(Sub, SUB_OP)
_OP2(Mul, MUL_OP)
_OP2(Div, DIV_OP)
_OP1(Neg, SUB_OP)

_OPF1(Sin, std::sin)
_OPF1(Cos, std::cos)
_OPF1(Tan, std::tan)
_OPF1(ASin, std::asin)
_OPF1(ACos, std::acos)
_OPF1(ATan, std::atan)
_OPF1(SinH, std::sinh)
_OPF1(CosH, std::cosh)
_OPF1(TanH, std::tanh)
_OPF1(ASinH, std::asinh)
_OPF1(ACosH, std::acosh)
_OPF1(ATanH, std::atanh)
_OPF1(Exp, std::exp)
_OPF1(Log, std::log)
_OPF1(Abs, std::abs)
_OPF1(Sqrt, std::sqrt)
_OPF1(Cbrt, std::cbrt)
_OPF1(Ceil, std::ceil)
_OPF1(Floor, std::floor)
_OPF1(Round, std::round)

_OPF2(ATan2, std::atan2)
_OPF2(Max, std::max)
_OPF2(Min, std::min)
_OPF2(Pow, std::pow)

class ScalarMathPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string& type_name, const SceneLoadContext& ctx) override
	{
		if (type_name == "add") {
			return std::make_shared<AddMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)),
											 ctx.lookupScalarNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "sub" || type_name == "subtract") {
			return std::make_shared<SubMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)),
											 ctx.lookupScalarNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "mul" || type_name == "multiply") {
			return std::make_shared<MulMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)),
											 ctx.lookupScalarNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "div" || type_name == "divide") {
			return std::make_shared<DivMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)),
											 ctx.lookupScalarNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "neg" || type_name == "negate") {
			return std::make_shared<NegMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "sin") {
			return std::make_shared<SinMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "cos") {
			return std::make_shared<CosMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "tan") {
			return std::make_shared<TanMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "asin") {
			return std::make_shared<ASinMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "acos") {
			return std::make_shared<ACosMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "atan") {
			return std::make_shared<ATanMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "sinh") {
			return std::make_shared<SinHMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "cosh") {
			return std::make_shared<CosHMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "tanh") {
			return std::make_shared<TanHMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "asinh") {
			return std::make_shared<ASinHMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "acosh") {
			return std::make_shared<ACosHMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "atanh") {
			return std::make_shared<ATanHMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "exp") {
			return std::make_shared<ExpMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "log") {
			return std::make_shared<LogMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "abs") {
			return std::make_shared<AbsMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "sqrt") {
			return std::make_shared<SqrtMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "cbrt") {
			return std::make_shared<CbrtMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "floor") {
			return std::make_shared<FloorMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "ceil") {
			return std::make_shared<CeilMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "round") {
			return std::make_shared<RoundMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "atan2") {
			return std::make_shared<ATan2Math>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)),
											   ctx.lookupScalarNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "max") {
			return std::make_shared<MaxMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)),
											 ctx.lookupScalarNode(ctx.parameters().getParameter(1)));
		} else if (type_name == "min") {
			return std::make_shared<MinMath>(ctx.lookupScalarNode(ctx.parameters().getParameter(0)),
											 ctx.lookupScalarNode(ctx.parameters().getParameter(1)));
		} else {
			PR_ASSERT(false, "ScalarMathNode plugin does not handle all offered types of operations!");
			return nullptr;
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "add", "sub", "subtract", "mul", "multiply", "div", "divide",
													  "neg", "negate",
													  "sin", "cos", "tan", "asin", "acos", "atan",
													  "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
													  "exp", "log", "abs", "sqrt", "cbrt", "ceil", "floor", "round",
													  "atan2", "max", "min" });
		return names;
	}

	PluginSpecification specification(const std::string& type_name) const override
	{
		PluginSpecificationBuilder builder("Scalar '" + type_name + "' Node", "A scalar math node");
		builder.Identifiers({ type_name });

		if (type_name == "add"
			|| type_name == "sub" || type_name == "subtract"
			|| type_name == "mul" || type_name == "multiply"
			|| type_name == "div" || type_name == "divide"
			|| type_name == "atan2" || type_name == "max" || type_name == "min")
			builder.Inputs().ScalarNode("op1", "First operand", 0.0f).ScalarNode("op2", "Second operand", 0.0f);
		else
			builder.Inputs().ScalarNode("op", "Operand", 0.0f);

		return builder.get();
	}

	
};
} // namespace PR

PR_PLUGIN_INIT(PR::ScalarMathPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)