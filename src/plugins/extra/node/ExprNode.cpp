#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/INodePlugin.h"

#include <SeExpr2/ExprFunc.h>
#include <SeExpr2/Expression.h>
#include <SeExpr2/Vec.h>

#include <fstream>

namespace PR {
constexpr char POSITION_VARIABLE[]	 = "p";
constexpr char WAVELENGTH_VARIABLE[] = "w";
constexpr char TEXTURE_VARIABLE[]	 = "uv";

struct ScalarVariable : public SeExpr2::ExprVarRef {
	ScalarVariable(bool isConst)
		: SeExpr2::ExprVarRef(isConst ? SeExpr2::ExprType().FP(1).Constant()
									  : SeExpr2::ExprType().FP(1).Varying())
		, Value{ 0 }
	{
	}

	float Value;

	void eval(double* result) { result[0] = Value; }
	void eval(const char**) { PR_ASSERT(false, "eval on string not implemented!"); }
};

struct SpectralVariable : public SeExpr2::ExprVarRef {
	SpectralVariable(bool isConst)
		: SeExpr2::ExprVarRef(isConst ? SeExpr2::ExprType().FP(1).Constant()
									  : SeExpr2::ExprType().FP(1).Varying())
		, Value(SpectralBlob::Zero())
		, CurrentEntry(0)
	{
	}

	SpectralBlob Value;
	int CurrentEntry;

	void eval(double* result) { result[0] = Value(CurrentEntry); }
	void eval(const char**) { PR_ASSERT(false, "eval on string not implemented!"); }
};

struct VectorVariable : public SeExpr2::ExprVarRef {
	VectorVariable(int d, bool isConst)
		: SeExpr2::ExprVarRef(isConst ? SeExpr2::ExprType().FP(d).Constant()
									  : SeExpr2::ExprType().FP(d).Varying())
		, Value{ 0, 0, 0 }
		, Dim(d)
	{
		PR_ASSERT(d > 0 && d <= 3, "Expected valid dimension");
	}

	float Value[3];
	const int Dim;

	void eval(double* result)
	{
		for (int i = 0; i < Dim; ++i)
			result[i] = Value[i];
	}

	void eval(const char**) { PR_ASSERT(false, "eval on string not implemented!"); }
};

class PrExpression : public SeExpr2::Expression {
public:
	PrExpression(const std::string& expr, bool isVec)
		: SeExpr2::Expression(expr, SeExpr2::ExprType().FP(isVec ? 3 : 1))
	{
	}

	PrExpression(PrExpression&& other) = default;

	std::unordered_map<ScalarVariable*, std::shared_ptr<FloatScalarNode>> ScalarNodes;
	std::unordered_map<SpectralVariable*, std::shared_ptr<FloatSpectralNode>> SpectralNodes;
	std::unordered_map<VectorVariable*, std::shared_ptr<FloatVectorNode>> VectorNodes;

	mutable std::unordered_map<std::string, std::unique_ptr<ScalarVariable>> ScalarVariableReferences;
	mutable std::unordered_map<std::string, std::unique_ptr<SpectralVariable>> SpectralVariableReferences;
	mutable std::unordered_map<std::string, std::unique_ptr<VectorVariable>> VectorVariableReferences;

	mutable std::unique_ptr<VectorVariable> PositionVariable;
	mutable std::unique_ptr<VectorVariable> TextureVariable;
	mutable std::unique_ptr<SpectralVariable> WavelengthVariable;

	SeExpr2::ExprVarRef* resolveVar(const std::string& name) const
	{
		if (POSITION_VARIABLE == name)
			return PositionVariable.get();
		else if (TEXTURE_VARIABLE == name)
			return TextureVariable.get();
		else if (WAVELENGTH_VARIABLE == name)
			return WavelengthVariable.get();
		else {
			const auto i1 = ScalarVariableReferences.find(name);
			if (i1 != ScalarVariableReferences.end())
				return i1->second.get();

			const auto i2 = SpectralVariableReferences.find(name);
			if (i2 != SpectralVariableReferences.end())
				return i2->second.get();

			const auto i3 = VectorVariableReferences.find(name);
			if (i3 != VectorVariableReferences.end())
				return i3->second.get();
			return nullptr;
		}
	}
};

// No Spectrals
inline static void updateBasicNodes(const ShadingContext& ctx, const std::shared_ptr<PrExpression>& expr)
{
	for (auto entry : expr->ScalarNodes)
		entry.first->Value = entry.second->eval(ctx);

	for (auto entry : expr->VectorNodes) {
		auto vec			  = entry.second->eval(ctx);
		entry.first->Value[0] = vec(0);
		entry.first->Value[1] = vec(1);
		entry.first->Value[2] = vec(2);
	}

	if (expr->TextureVariable) {
		expr->TextureVariable->Value[0] = ctx.UV[0];
		expr->TextureVariable->Value[1] = ctx.UV[1];
	}
}

class ScalarExpressionNode : public FloatScalarNode {
public:
	explicit ScalarExpressionNode(const std::shared_ptr<PrExpression>& expr)
		: mExpr(expr)
	{
	}

	float eval(const ShadingContext& ctx) const override
	{
		// Update nodes
		updateBasicNodes(ctx, mExpr);

		const double* result = mExpr->evalFP();
		return result[0];
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Scalar Expression (" << mExpr->getExpr() << ")";
		return sstream.str();
	}

private:
	std::shared_ptr<PrExpression> mExpr;
};

class SpectralExpressionNode : public FloatSpectralNode {
public:
	explicit SpectralExpressionNode(const std::shared_ptr<PrExpression>& expr)
		: mExpr(expr)
	{
	}

	// TODO: Better way?
	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		// Update nodes
		updateBasicNodes(ctx, mExpr);

		// Update spectral nodes
		for (auto entry : mExpr->SpectralNodes)
			entry.first->Value = entry.second->eval(ctx);

		if (mExpr->WavelengthVariable)
			mExpr->WavelengthVariable->Value = ctx.WavelengthNM;

		// Calculate each wavelength (not really the best solution)
		SpectralBlob output;
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			for (auto entry : mExpr->SpectralNodes)
				entry.first->CurrentEntry = i;

			if (mExpr->WavelengthVariable)
				mExpr->WavelengthVariable->CurrentEntry = i;

			const double* result = mExpr->evalFP();
			output[i]			 = result[0];
		}
		return output;
	}

	// TODO: Get a better way to achieve this!
	Vector2i queryRecommendedSize() const override
	{
		return Vector2i(1, 1);
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Spectral Expression (" << mExpr->getExpr() << ")";
		return sstream.str();
	}

private:
	std::shared_ptr<PrExpression> mExpr;
};

class VectorExpressionNode : public FloatVectorNode {
public:
	explicit VectorExpressionNode(const std::shared_ptr<PrExpression>& expr)
		: mExpr(expr)
	{
	}

	Vector3f eval(const ShadingContext& ctx) const override
	{
		// Update nodes
		updateBasicNodes(ctx, mExpr);

		const double* result = mExpr->evalFP();
		return Vector3f(result[0], result[1], result[2]);
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Vector3f Expression (" << mExpr->getExpr() << ")";
		return sstream.str();
	}

private:
	std::shared_ptr<PrExpression> mExpr;
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
				PR_LOG(L_ERROR) << "[SeExpr2] Can not load file " << path << std::endl;
				return nullptr;
			}
			expr_str = std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
		}

		const bool isVec = (type_name == "vexpr" || type_name == "fvexpr");
		auto expr		 = std::make_shared<PrExpression>(expr_str, isVec);

		// Setup user variables
		for (const auto& entry : ctx.Parameters.parameters()) {
			const auto& param = entry.second;

			switch (param.type()) {
			case PT_Invalid:
				continue;
			case PT_Bool: {
				std::unique_ptr<ScalarVariable> constant	= std::make_unique<ScalarVariable>(true);
				constant->Value								= param.getBool(false) ? 1.0f : 0.0f;
				expr->ScalarVariableReferences[entry.first] = std::move(constant);
			} break;
			case PT_Int:
			case PT_UInt:
			case PT_Number: {
				std::unique_ptr<ScalarVariable> constant	= std::make_unique<ScalarVariable>(true);
				constant->Value								= param.getNumber(0.0f);
				expr->ScalarVariableReferences[entry.first] = std::move(constant);
			} break;
			case PT_String:
			case PT_Reference: {
				auto node = ctx.Env->lookupRawNode(param);
				switch (node->type()) {
				case NT_FloatScalar: {
					auto var									= std::make_unique<ScalarVariable>(false);
					expr->ScalarNodes[var.get()]				= std::reinterpret_pointer_cast<FloatScalarNode>(node);
					expr->ScalarVariableReferences[entry.first] = std::move(var);
				} break;
				case NT_FloatSpectral: {
					auto var									  = std::make_unique<SpectralVariable>(false);
					expr->SpectralNodes[var.get()]				  = std::reinterpret_pointer_cast<FloatSpectralNode>(node);
					expr->SpectralVariableReferences[entry.first] = std::move(var);
				} break;
				case NT_FloatVector: {
					auto var									= std::make_unique<VectorVariable>(3, false);
					expr->VectorNodes[var.get()]				= std::reinterpret_pointer_cast<FloatVectorNode>(node);
					expr->VectorVariableReferences[entry.first] = std::move(var);
				} break;
				}
			} break;
			}
		}

		// Setup standard variables
		if (expr->usesVar(POSITION_VARIABLE))
			expr->PositionVariable = std::make_unique<VectorVariable>(3, false);
		if (expr->usesVar(TEXTURE_VARIABLE))
			expr->TextureVariable = std::make_unique<VectorVariable>(2, false);
		if (expr->usesVar(WAVELENGTH_VARIABLE))
			expr->WavelengthVariable = std::make_unique<SpectralVariable>(false);

		// const bool isSpatialVarying	 = expr->PositionVariable || expr->TextureVariable;
		const bool isSpectralVarying = (expr->WavelengthVariable != nullptr) || !expr->SpectralVariableReferences.empty();

		if (!expr->isValid()) {
			PR_LOG(L_ERROR) << "[SeExpr2] Parsing Error: " << expr->parseError() << std::endl;
			return nullptr;
		}

		if (isSpectralVarying && isVec) {
			PR_LOG(L_ERROR) << "[SeExpr2] A spectral varying expression can not be combined with a vector expression" << std::endl;
			return nullptr;
		}

		if (isVec) {
			return std::make_shared<VectorExpressionNode>(expr);
		} else if (isSpectralVarying) {
			return std::make_shared<SpectralExpressionNode>(expr);
		} else {
			return std::make_shared<ScalarExpressionNode>(expr);
		}
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