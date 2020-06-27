#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/INodePlugin.h"

#include <SeExpr2/ExprFunc.h>
#include <SeExpr2/Expression.h>
#include <SeExpr2/Vec.h>

#include <fstream>

namespace PR {
struct Variable : public SeExpr2::ExprVarRef {
	Variable(bool isVec, bool isConst)
		: SeExpr2::ExprVarRef(isConst ? SeExpr2::ExprType().FP(isVec ? 3 : 1).Constant()
									  : SeExpr2::ExprType().FP(isVec ? 3 : 1).Varying())
		, Value{ 0, 0, 0 }
		, IsVec(isVec)
	{
	}

	float Value[3];
	bool IsVec;

	void eval(double* result)
	{
		result[0] = Value[0];
		if (IsVec) {
			result[1] = Value[1];
			result[2] = Value[2];
		}
	}

	void eval(const char**) { PR_ASSERT(false, "eval on string not implemented!"); }
};

class PrExpression : public SeExpr2::Expression {
public:
	PrExpression(const std::string& expr, bool isVec)
		: SeExpr2::Expression(expr, SeExpr2::ExprType().FP(isVec ? 3 : 1))
	{
	}

	mutable std::unordered_map<std::string, Variable> VariableReferences;

	SeExpr2::ExprVarRef* resolveVar(const std::string& name) const
	{
		std::unordered_map<std::string, Variable>::iterator i = VariableReferences.find(name);
		if (i != VariableReferences.end())
			return &i->second;
		return 0;
	}
};
class ExpressionPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(uint32, const std::string& type_name, const SceneLoadContext& ctx) override
	{
		std::string expr_str;
		if (type_name == "expr" || type_name == "vexpr") {
			expr_str = ctx.Parameters.getString("expression", "");
			if (expr_str.empty())
				expr_str = ctx.Parameters.getString(0, "");
		} else {
			std::string path = ctx.Parameters.getString("file", "");
			if (path.empty())
				path = ctx.Parameters.getString(0, "");

			std::ifstream stream(path, std::ios::in);
			if (!stream.good()) {
				PR_LOG(L_ERROR) << "Can not load file " << path << std::endl;
				return nullptr;
			}
			expr_str = std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
		}

		const bool isVec = (type_name == "vexpr" || type_name == "fvexpr");

		std::unordered_map<std::string, std::shared_ptr<FloatSpectralNode>> spectral_vars;

		SeExpr2::Expression expr(expr_str, SeExpr2::ExprType().FP(isVec ? 3 : 1));

		for (const auto& entry : ctx.Parameters.parameters()) {
			if (expr.usesVar(entry.first))
				spectral_vars[entry.first] = ctx.Env->lookupSpectralNode(entry.second);
		}

		if (!expr.syntaxOK()) {
			PR_LOG(L_ERROR) << "[SeExpr2] Parsing Error: " << expr.parseError() << std::endl;
			return nullptr;
		}

		return nullptr;
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "expr", "vexpr", "fexpr", "fvexpr" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::ExpressionPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)