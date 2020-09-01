#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "renderer/RenderContext.h"
#include "shader/INodePlugin.h"

#include <SeExpr2/ExprFunc.h>
#include <SeExpr2/Expression.h>
#include <SeExpr2/VarBlock.h>
#include <SeExpr2/Vec.h>

#include <fstream>
#include <regex>

namespace PR {
constexpr char POSITION_VARIABLE[]	 = "P";
constexpr char WAVELENGTH_VARIABLE[] = "w";// TODO: Better identifier?
constexpr char TEXTURE_U_VARIABLE[]	 = "u";
constexpr char TEXTURE_V_VARIABLE[]	 = "v";

static inline bool usesVariable(const std::string& expr, const std::string& var)
{
	return std::regex_search(expr, std::regex("\\b" + var + "\\b"));
}

struct ExpressionContainer {
	SeExpr2::Expression Expr;
	SeExpr2::VarBlockCreator Creator;
	std::vector<SeExpr2::VarBlock> VarBlocks;
	std::vector<std::vector<double>> VarData;

	std::vector<std::pair<int, std::shared_ptr<FloatScalarNode>>> ScalarNodes;
	std::vector<std::pair<int, std::pair<SpectralBlob, std::shared_ptr<FloatSpectralNode>>>> SpectralNodes;
	std::vector<std::pair<int, std::shared_ptr<FloatVectorNode>>> VectorNodes;
	std::vector<std::pair<int, float>> NumberConstants;
	std::vector<std::pair<int, Vector3f>> VectorConstants;

	int PositionVariable   = -1;
	int WavelengthVariable = -1;
	int TextureUVariable   = -1;
	int TextureVVariable   = -1;

	inline ExpressionContainer(const std::string& str, bool isVec)
		: Expr(str)
	{
		Expr.setDesiredReturnType(SeExpr2::TypeVec(isVec ? 3 : 1));
		Expr.setVarBlockCreator(&Creator);

		// Setup standard variables
		if (usesVariable(str, POSITION_VARIABLE))
			PositionVariable = Creator.registerVariable(POSITION_VARIABLE,
														SeExpr2::ExprType().FP(3).Varying());
		if (usesVariable(str, TEXTURE_U_VARIABLE))
			TextureUVariable = Creator.registerVariable(TEXTURE_U_VARIABLE,
														SeExpr2::ExprType().FP(1).Varying());
		if (usesVariable(str, TEXTURE_V_VARIABLE))
			TextureVVariable = Creator.registerVariable(TEXTURE_V_VARIABLE,
														SeExpr2::ExprType().FP(1).Varying());
		if (usesVariable(str, WAVELENGTH_VARIABLE))
			WavelengthVariable = Creator.registerVariable(WAVELENGTH_VARIABLE,
														  SeExpr2::ExprType().FP(1).Varying());
	}

	inline void updateConstants(SeExpr2::VarBlock* block) const
	{
		for (const auto& entry : NumberConstants)
			*block->Pointer(entry.first) = entry.second;

		for (const auto& entry : VectorConstants) {
			Vector3f var = entry.second;
			auto* ptr	 = block->Pointer(entry.first);
			ptr[0]		 = var(0);
			ptr[1]		 = var(1);
			ptr[2]		 = var(2);
		}
	}

	inline void setupThreadData(size_t thread_count)
	{
		for (size_t i = 0; i < thread_count; ++i) {
			SeExpr2::VarBlock block = Creator.create(true);
			setupVarData(&block);
			updateConstants(&block);
			VarBlocks.push_back(std::move(block));
		}
	}

	inline void setupVarData(SeExpr2::VarBlock* block)
	{
		// Calculate entry count
		size_t entries = 0;
		if (PositionVariable >= 0)
			entries += 3;
		if (TextureUVariable >= 0)
			entries += 1;
		if (TextureVVariable >= 0)
			entries += 1;
		if (WavelengthVariable >= 0)
			entries += 1;
		entries += NumberConstants.size();
		entries += 3 * VectorConstants.size();
		entries += ScalarNodes.size();
		entries += SpectralNodes.size();
		entries += 3 * VectorNodes.size();

		// Setup data
		VarData.emplace_back(entries);
		std::vector<double>& data = VarData.back();

		// Connect pointers to slots given by the handles (which are just array indices)
		if (PositionVariable >= 0)
			block->Pointer(PositionVariable) = &data[PositionVariable];
		if (TextureUVariable >= 0)
			block->Pointer(TextureUVariable) = &data[TextureUVariable];
		if (TextureVVariable >= 0)
			block->Pointer(TextureVVariable) = &data[TextureVVariable];
		if (WavelengthVariable >= 0)
			block->Pointer(WavelengthVariable) = &data[WavelengthVariable];

		for (const auto& entry : NumberConstants)
			block->Pointer(entry.first) = &data[entry.first];

		for (const auto& entry : VectorConstants)
			block->Pointer(entry.first) = &data[entry.first];

		for (const auto& entry : ScalarNodes)
			block->Pointer(entry.first) = &data[entry.first];

		for (const auto& entry : SpectralNodes)
			block->Pointer(entry.first) = &data[entry.first];

		for (const auto& entry : VectorNodes)
			block->Pointer(entry.first) = &data[entry.first];
	}

	inline void updateVaryings(const ShadingContext& ctx)
	{
		PR_ASSERT(ctx.ThreadIndex < VarBlocks.size(), "Invalid thread index");
		auto* varblock = &VarBlocks[ctx.ThreadIndex];

		for (const auto& entry : ScalarNodes)
			*varblock->Pointer(entry.first) = entry.second->eval(ctx);

		for (auto& entry : SpectralNodes)
			entry.second.first = entry.second.second->eval(ctx);

		for (const auto& entry : VectorNodes) {
			Vector3f var = entry.second->eval(ctx);
			auto* ptr	 = varblock->Pointer(entry.first);
			ptr[0]		 = var(0);
			ptr[1]		 = var(1);
			ptr[2]		 = var(2);
		}

		/*if (PositionVariable >= 0) {
			auto* ptr = VarBlock->Pointer(PositionVariable);
			ptr[0]	  = ctx.Position[0];
			ptr[1]	  = ctx.Position[1];
			ptr[2]	  = ctx.Position[2];
		}*/

		if (TextureUVariable >= 0)
			*varblock->Pointer(TextureUVariable) = ctx.UV(0);

		if (TextureVVariable >= 0)
			*varblock->Pointer(TextureVVariable) = ctx.UV(1);
	}

	inline void updateSpectralVaryings(size_t i, const ShadingContext& ctx)
	{
		PR_ASSERT(ctx.ThreadIndex < VarBlocks.size(), "Invalid thread index");

		auto* varblock = &VarBlocks[ctx.ThreadIndex];
		for (auto& entry : SpectralNodes)
			*varblock->Pointer(entry.first) = entry.second.first[i];

		if (WavelengthVariable >= 0)
			*varblock->Pointer(WavelengthVariable) = ctx.WavelengthNM[i];
	}

	inline void registerConstant(const std::string& name, float constant)
	{
		int handle = Creator.registerVariable(name, SeExpr2::ExprType().FP(1).Constant());
		NumberConstants.emplace_back(handle, constant);
	}

	inline void registerConstant(const std::string& name, const Vector3f& constant)
	{
		int handle = Creator.registerVariable(name, SeExpr2::ExprType().FP(3).Constant());
		VectorConstants.emplace_back(handle, constant);
	}

	inline void registerVarying(const std::string& name, const std::shared_ptr<FloatScalarNode>& node)
	{
		int handle = Creator.registerVariable(name, SeExpr2::ExprType().FP(1).Varying());
		ScalarNodes.emplace_back(handle, node);
	}

	inline void registerVarying(const std::string& name, const std::shared_ptr<FloatSpectralNode>& node)
	{
		int handle = Creator.registerVariable(name, SeExpr2::ExprType().FP(1).Varying());
		SpectralNodes.emplace_back(handle, std::make_pair(SpectralBlob(), node));
	}

	inline void registerVarying(const std::string& name, const std::shared_ptr<FloatVectorNode>& node)
	{
		int handle = Creator.registerVariable(name, SeExpr2::ExprType().FP(3).Varying());
		VectorNodes.emplace_back(handle, node);
	}

	inline bool isSpectralVarying() const
	{
		return WavelengthVariable >= 0 || !SpectralNodes.empty();
	}
};

class ScalarExpressionNode : public FloatScalarNode {
public:
	explicit ScalarExpressionNode(const std::shared_ptr<ExpressionContainer>& expr)
		: mExpr(expr)
	{
	}

	float eval(const ShadingContext& ctx) const override
	{
		mExpr->updateVaryings(ctx);
		const double* result = mExpr->Expr.evalFP(&mExpr->VarBlocks[ctx.ThreadIndex]);
		return result[0];
	}

	void beforeRender(RenderContext* ctx) override
	{
		FloatScalarNode::beforeRender(ctx);
		mExpr->setupThreadData(ctx->threads());
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Scalar Expression (" << mExpr->Expr.getExpr() << ")";
		return sstream.str();
	}

private:
	mutable std::shared_ptr<ExpressionContainer> mExpr;
};

class SpectralExpressionNode : public FloatSpectralNode {
public:
	explicit SpectralExpressionNode(const std::shared_ptr<ExpressionContainer>& expr)
		: mExpr(expr)
	{
	}

	// TODO: Better way?
	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		mExpr->updateVaryings(ctx);

		// Calculate each wavelength (not really the best solution)
		SpectralBlob output;
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			mExpr->updateSpectralVaryings(i, ctx);

			const double* result = mExpr->Expr.evalFP(&mExpr->VarBlocks[ctx.ThreadIndex]);
			output[i]			 = result[0];
		}
		return output;
	}

	// TODO: Get a better way to achieve this!
	Vector2i queryRecommendedSize() const override
	{
		return Vector2i(1, 1);
	}

	void beforeRender(RenderContext* ctx) override
	{
		FloatSpectralNode::beforeRender(ctx);
		mExpr->setupThreadData(ctx->threads());
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Spectral Expression (" << mExpr->Expr.getExpr() << ")";
		return sstream.str();
	}

private:
	mutable std::shared_ptr<ExpressionContainer> mExpr;
};

class VectorExpressionNode : public FloatVectorNode {
public:
	explicit VectorExpressionNode(const std::shared_ptr<ExpressionContainer>& expr)
		: mExpr(expr)
	{
	}

	Vector3f eval(const ShadingContext& ctx) const override
	{
		mExpr->updateVaryings(ctx);
		const double* result = mExpr->Expr.evalFP(&mExpr->VarBlocks[ctx.ThreadIndex]);
		return Vector3f(result[0], result[1], result[2]);
	}

	void beforeRender(RenderContext* ctx) override
	{
		FloatVectorNode::beforeRender(ctx);
		mExpr->setupThreadData(ctx->threads());
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "Vector3f Expression (" << mExpr->Expr.getExpr() << ")";
		return sstream.str();
	}

private:
	mutable std::shared_ptr<ExpressionContainer> mExpr;
};

class ExpressionPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(uint32, const std::string& type_name, const SceneLoadContext& ctx) override
	{
		std::string expr_str;
		if (type_name == "expr" || type_name == "vexpr") {
			expr_str = ctx.parameters().getString("expression", "");
			if (expr_str.empty())
				expr_str = ctx.parameters().getString(0, "");
		} else {
			std::string path = ctx.parameters().getString("file", "");
			if (path.empty())
				path = ctx.parameters().getString(0, "");

			std::ifstream stream(path, std::ios::in);
			if (!stream.good()) {
				PR_LOG(L_ERROR) << "[SeExpr2] Can not load file " << path << std::endl;
				return nullptr;
			}
			expr_str = std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
		}

		const bool isVec = (type_name == "vexpr" || type_name == "fvexpr");
		auto expr		 = std::make_shared<ExpressionContainer>(expr_str, isVec);

		// Setup user variables
		for (const auto& entry : ctx.parameters().parameters()) {
			const auto& param = entry.second;

			switch (param.type()) {
			case PT_Invalid:
				continue;
			case PT_Bool:
				expr->registerConstant(entry.first, param.getBool(false) ? 1.0f : 0.0f);
				break;
			case PT_Int:
			case PT_UInt:
			case PT_Number:
				expr->registerConstant(entry.first, param.getNumber(0.0f));
				break;
			case PT_String:
			case PT_Reference: {
				auto node = ctx.Env->lookupRawNode(param);
				switch (node->type()) {
				case NT_FloatScalar:
					expr->registerVarying(entry.first, std::reinterpret_pointer_cast<FloatScalarNode>(node));
					break;
				case NT_FloatSpectral:
					expr->registerVarying(entry.first, std::reinterpret_pointer_cast<FloatSpectralNode>(node));
					break;
				case NT_FloatVector:
					expr->registerVarying(entry.first, std::reinterpret_pointer_cast<FloatVectorNode>(node));
					break;
				}
			} break;
			}
		}

		if (!expr->Expr.isValid()) {
			PR_LOG(L_ERROR) << "[SeExpr2] Parsing Error: " << expr->Expr.parseError() << std::endl;
			return nullptr;
		}

		if (expr->isSpectralVarying() && isVec) {
			PR_LOG(L_ERROR) << "[SeExpr2] A spectral varying expression can not be combined with a vector expression" << std::endl;
			return nullptr;
		}

		if (isVec) {
			return std::make_shared<VectorExpressionNode>(expr);
		} else if (expr->isSpectralVarying()) {
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