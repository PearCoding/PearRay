#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/INodePlugin.h"

namespace PR {
enum ScaleType {
	ST_None,
	ST_Isotropic,
	ST_Anisotropic
};

template <ScaleType ST>
class CheckerboardNode : public FloatSpectralNode {
public:
	explicit CheckerboardNode(const std::shared_ptr<FloatSpectralNode>& op1, const std::shared_ptr<FloatSpectralNode>& op2,
							  const std::shared_ptr<FloatScalarNode>& scaleU, const std::shared_ptr<FloatScalarNode>& scaleV)
		: mOp1(op1)
		, mOp2(op2)
		, mScaleU(scaleU)
		, mScaleV(scaleV)
	{
	}

	inline static bool check(const Vector2f& uv) { return (uv[0] >= 0.5f) != (uv[1] >= 0.5f); }

	inline bool check(const ShadingContext& ctx) const
	{
		if constexpr (ST == ST_None)
			return check(ctx.UV);
		else if constexpr (ST == ST_Isotropic) {
			const float scale = mScaleU->eval(ctx);
			return check(ctx.UV / scale);
		} else {
			const float scaleU = mScaleU->eval(ctx);
			const float scaleV = mScaleV->eval(ctx);
			return check(ctx.UV.array() / Vector2f(scaleU, scaleV).array());
		}
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		if (check(ctx))
			return mOp2->eval(ctx);
		else
			return mOp1->eval(ctx);
	}

	Vector2i queryRecommendedSize() const override
	{
		return mOp1->queryRecommendedSize().cwiseMax(mOp2->queryRecommendedSize());
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "CheckerboardNode (" << mOp1->dumpInformation()
				<< ", " << mOp2->dumpInformation()
				<< ", " << mScaleU->dumpInformation()
				<< ", " << mScaleV->dumpInformation() << ")";
		return sstream.str();
	}

private:
	const std::shared_ptr<FloatSpectralNode> mOp1;
	const std::shared_ptr<FloatSpectralNode> mOp2;
	const std::shared_ptr<FloatScalarNode> mScaleU;
	const std::shared_ptr<FloatScalarNode> mScaleV;
};

class CheckerboardPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(uint32, const std::string&, const SceneLoadContext& ctx) override
	{
		const auto op1	  = ctx.lookupSpectralNode(ctx.parameters().getParameter(0), 1);
		const auto op2	  = ctx.lookupSpectralNode(ctx.parameters().getParameter(1), 0);
		const auto scaleU = ctx.lookupScalarNode(ctx.parameters().getParameter(2), 1);
		const auto scaleV = ctx.lookupScalarNode(ctx.parameters().getParameter(3), 1);

		if (ctx.parameters().positionalParameterCount() == 2)
			return std::make_shared<CheckerboardNode<ST_None>>(op1, op2, scaleU, scaleU);
		else if (ctx.parameters().positionalParameterCount() == 3)
			return std::make_shared<CheckerboardNode<ST_Isotropic>>(op1, op2, scaleU, scaleU);
		else
			return std::make_shared<CheckerboardNode<ST_Anisotropic>>(op1, op2, scaleU, scaleV);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "grid", "checkerboard" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::CheckerboardPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)