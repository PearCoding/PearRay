#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/INodePlugin.h"

namespace PR {
enum class ScaleType {
	None,
	Isotropic,
	Anisotropic
};

template <ScaleType ST>
class CheckerboardNode : public FloatSpectralNode {
public:
	explicit CheckerboardNode(const std::shared_ptr<FloatSpectralNode>& op1, const std::shared_ptr<FloatSpectralNode>& op2,
							  const std::shared_ptr<FloatScalarNode>& scaleU, const std::shared_ptr<FloatScalarNode>& scaleV)
		: FloatSpectralNode(op1->flags() | op2->flags() | scaleU->flags() | (scaleV->flags() | NodeFlag::TextureVarying))
		, mOp1(op1)
		, mOp2(op2)
		, mScaleU(scaleU)
		, mScaleV(scaleV)
	{
	}

	inline static bool check(const Vector2f& uv) { return ((int)std::floor(uv[0]) + (int)std::floor(uv[1])) % 2 == 0; }

	inline bool check(const ShadingContext& ctx) const
	{
		if constexpr (ST == ScaleType::None) {
			return check(ctx.UV);
		} else if constexpr (ST == ScaleType::Isotropic) {
			const float scale = mScaleU->eval(ctx);
			return check(ctx.UV * scale);
		} else {
			const float scaleU = mScaleU->eval(ctx);
			const float scaleV = mScaleV->eval(ctx);
			return check(ctx.UV.array() * Vector2f(scaleU, scaleV).array());
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

	SpectralRange spectralRange() const override { return mOp1->spectralRange() + mOp2->spectralRange(); }

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
	std::shared_ptr<INode> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const auto op1	  = ctx.lookupSpectralNode(ctx.parameters().getParameter(0), 0.8f);
		const auto op2	  = ctx.lookupSpectralNode(ctx.parameters().getParameter(1), 0.2f);
		const auto scaleU = ctx.lookupScalarNode(ctx.parameters().getParameter(2), 5);
		const auto scaleV = ctx.lookupScalarNode(ctx.parameters().getParameter(3), 5);

		if (ctx.parameters().positionalParameterCount() == 2)
			return std::make_shared<CheckerboardNode<ScaleType::None>>(op1, op2, scaleU, scaleU);
		else if (ctx.parameters().positionalParameterCount() == 3)
			return std::make_shared<CheckerboardNode<ScaleType::Isotropic>>(op1, op2, scaleU, scaleU);
		else
			return std::make_shared<CheckerboardNode<ScaleType::Anisotropic>>(op1, op2, scaleU, scaleV);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "grid", "checkerboard" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Checkerboard Node", "A cheeky checkerboard node")
			.Identifiers(getNames())
			.Inputs()
			.SpectralNode("color1", "First color node", 0.8f)
			.SpectralNode("color2", "Second color node", 0.2f)
			.SpectralNode("scale_u", "U scale", 5)
			.SpectralNode("scale_v", "V scale", 5)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::CheckerboardPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)