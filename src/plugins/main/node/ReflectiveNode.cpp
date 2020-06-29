#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "math/Reflection.h"
#include "shader/INodePlugin.h"

namespace PR {

inline float teval(const ShadingContext&, float a) { return a; }
inline SpectralBlob teval(const ShadingContext& ctx, const std::shared_ptr<FloatSpectralNode>& a) { return a->eval(ctx); }

inline std::string tdump(float a) { return std::to_string(a); }
inline std::string tdump(const std::shared_ptr<FloatSpectralNode>& a) { return a->dumpInformation(); }

template <typename T>
class Cauchy2IndexNode : public FloatSpectralNode {
public:
	explicit Cauchy2IndexNode(const T& a, const T& b)
		: mA(a)
		, mB(b)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override { return Reflection::cauchy(ctx.WavelengthNM, teval(ctx, mA), teval(ctx, mB)); }
	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "CauchyIndex (" << tdump(mA) << ", " << tdump(mB) << ")";
		return sstream.str();
	}

private:
	const T mA;
	const T mB;
};

template <typename T>
class Cauchy3IndexNode : public FloatSpectralNode {
public:
	explicit Cauchy3IndexNode(const T& a, const T& b, const T& c)
		: mA(a)
		, mB(b)
		, mC(c)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		return Reflection::cauchy(ctx.WavelengthNM, teval(ctx, mA), teval(ctx, mB), teval(ctx, mC));
	}

	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "CauchyIndex (" << tdump(mA) << ", " << tdump(mB) << ", " << tdump(mC) << ")";
		return sstream.str();
	}

private:
	const T mA;
	const T mB;
	const T mC;
};

template <typename T>
class SellmeierSqrIndexNode : public FloatSpectralNode {
public:
	explicit SellmeierSqrIndexNode(const T& b1, const T& b2, const T& b3, const T& c1, const T& c2, const T& c3)
		: mB1(b1)
		, mB2(b2)
		, mB3(b3)
		, mC1(c1)
		, mC2(c2)
		, mC3(c3)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		return Reflection::sellmeier2(ctx.WavelengthNM,
									  teval(ctx, mB1), teval(ctx, mB2), teval(ctx, mB3),
									  teval(ctx, mC1), teval(ctx, mC2), teval(ctx, mC3));
	}
	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "SquaredSellmeierIndex (" << tdump(mB1) << ", " << tdump(mB2) << ", " << tdump(mB3) << ", " << tdump(mC1) << ", " << tdump(mC2) << ", " << tdump(mC3) << ")";
		return sstream.str();
	}

private:
	const T mB1;
	const T mB2;
	const T mB3;
	const T mC1;
	const T mC2;
	const T mC3;
};

template <typename T>
class SellmeierIndexNode : public FloatSpectralNode {
public:
	explicit SellmeierIndexNode(const T& b1, const T& b2, const T& b3, const T& c1, const T& c2, const T& c3)
		: mB1(b1)
		, mB2(b2)
		, mB3(b3)
		, mC1(c1)
		, mC2(c2)
		, mC3(c3)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		return Reflection::sellmeier2(ctx.WavelengthNM,
									  teval(ctx, mB1), teval(ctx, mB2), teval(ctx, mB3),
									  teval(ctx, mC1), teval(ctx, mC2), teval(ctx, mC3))
			.cwiseSqrt();
	}
	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "SellmeierIndex (" << tdump(mB1) << ", " << tdump(mB2) << ", " << tdump(mB3) << ", " << tdump(mC1) << ", " << tdump(mC2) << ", " << tdump(mC3) << ")";
		return sstream.str();
	}

private:
	const T mB1;
	const T mB2;
	const T mB3;
	const T mC1;
	const T mC2;
	const T mC3;
};

template <typename T>
class PolySqrIndexNode : public FloatSpectralNode {
public:
	explicit PolySqrIndexNode(const T& a, const T& b1, const T& b2, const T& c1, const T& c2)
		: mA(a)
		, mB1(b1)
		, mB2(b2)
		, mC1(c1)
		, mC2(c2)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		return Reflection::poly2(ctx.WavelengthNM, teval(ctx, mA),
								 teval(ctx, mB1), teval(ctx, mB2),
								 teval(ctx, mC1), teval(ctx, mC2));
	}
	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "SquaredPolyIndex (" << tdump(mA) << ", " << tdump(mB1) << ", " << tdump(mB2) << ", " << tdump(mC1) << ", " << tdump(mC2) << ")";
		return sstream.str();
	}

private:
	const T mA;
	const T mB1;
	const T mB2;
	const T mC1;
	const T mC2;
};

template <typename T>
class PolyIndexNode : public FloatSpectralNode {
public:
	explicit PolyIndexNode(const T& a, const T& b1, const T& b2, const T& c1, const T& c2)
		: mA(a)
		, mB1(b1)
		, mB2(b2)
		, mC1(c1)
		, mC2(c2)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		return Reflection::poly2(ctx.WavelengthNM, teval(ctx, mA),
								 teval(ctx, mB1), teval(ctx, mB2),
								 teval(ctx, mC1), teval(ctx, mC2))
			.cwiseSqrt();
	}
	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "PolyIndex (" << tdump(mA) << ", " << tdump(mB1) << ", " << tdump(mB2) << ", " << tdump(mC1) << ", " << tdump(mC2) << ")";
		return sstream.str();
	}

private:
	const T mA;
	const T mB1;
	const T mB2;
	const T mC1;
	const T mC2;
};

using ParamNode = std::shared_ptr<FloatSpectralNode>;
class ReflectivePlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(uint32, const std::string& type_name, const SceneLoadContext& ctx) override
	{
		bool allNumber = true;
		for (const auto& param : ctx.Parameters.positionalParameters()) {
			if (!param.canBeNumber()) {
				allNumber = false;
				break;
			}
		}

		if (type_name == "cauchy_index") {
			if (allNumber) {
				if (ctx.Parameters.positionalParameters().size() == 3)
					return std::make_shared<Cauchy3IndexNode<float>>(ctx.Parameters.getParameter(0).getNumber(0.0f),
																	 ctx.Parameters.getParameter(1).getNumber(0.0f),
																	 ctx.Parameters.getParameter(2).getNumber(0.0f));
				else
					return std::make_shared<Cauchy2IndexNode<float>>(ctx.Parameters.getParameter(0).getNumber(0.0f),
																	 ctx.Parameters.getParameter(1).getNumber(0.0f));
			} else {
				if (ctx.Parameters.positionalParameters().size() == 3)
					return std::make_shared<Cauchy3IndexNode<ParamNode>>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
																		 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)),
																		 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(2)));
				else
					return std::make_shared<Cauchy2IndexNode<ParamNode>>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
																		 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)));
			}
		} else if (type_name == "sellmeier_index") {
			if (allNumber)
				return std::make_shared<SellmeierIndexNode<float>>(ctx.Parameters.getParameter(0).getNumber(0.0f),
																   ctx.Parameters.getParameter(1).getNumber(0.0f),
																   ctx.Parameters.getParameter(2).getNumber(0.0f),
																   ctx.Parameters.getParameter(3).getNumber(0.0f),
																   ctx.Parameters.getParameter(4).getNumber(0.0f),
																   ctx.Parameters.getParameter(5).getNumber(0.0f));
			else
				return std::make_shared<SellmeierIndexNode<ParamNode>>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
																	   ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)),
																	   ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(2)),
																	   ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(3)),
																	   ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(4)),
																	   ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(5)));
		} else if (type_name == "sellmeier2_index") {
			if (allNumber)
				return std::make_shared<SellmeierSqrIndexNode<float>>(ctx.Parameters.getParameter(0).getNumber(0.0f),
																	  ctx.Parameters.getParameter(1).getNumber(0.0f),
																	  ctx.Parameters.getParameter(2).getNumber(0.0f),
																	  ctx.Parameters.getParameter(3).getNumber(0.0f),
																	  ctx.Parameters.getParameter(4).getNumber(0.0f),
																	  ctx.Parameters.getParameter(5).getNumber(0.0f));
			else
				return std::make_shared<SellmeierSqrIndexNode<ParamNode>>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
																		  ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)),
																		  ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(2)),
																		  ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(3)),
																		  ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(4)),
																		  ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(5)));
		} else if (type_name == "poly_index") {
			if (allNumber)
				return std::make_shared<PolyIndexNode<float>>(ctx.Parameters.getParameter(0).getNumber(0.0f),
															  ctx.Parameters.getParameter(1).getNumber(0.0f),
															  ctx.Parameters.getParameter(2).getNumber(0.0f),
															  ctx.Parameters.getParameter(3).getNumber(0.0f),
															  ctx.Parameters.getParameter(4).getNumber(0.0f));
			else
				return std::make_shared<PolyIndexNode<ParamNode>>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
																  ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)),
																  ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(2)),
																  ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(3)),
																  ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(4)));
		} else if (type_name == "poly2_index") {
			if (allNumber)
				return std::make_shared<PolySqrIndexNode<float>>(ctx.Parameters.getParameter(0).getNumber(0.0f),
																 ctx.Parameters.getParameter(1).getNumber(0.0f),
																 ctx.Parameters.getParameter(2).getNumber(0.0f),
																 ctx.Parameters.getParameter(3).getNumber(0.0f),
																 ctx.Parameters.getParameter(4).getNumber(0.0f));
			else
				return std::make_shared<PolySqrIndexNode<ParamNode>>(ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(0)),
																	 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(1)),
																	 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(2)),
																	 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(3)),
																	 ctx.Env->lookupSpectralNode(ctx.Parameters.getParameter(4)));
		} else {
			PR_ASSERT(false, "ReflectiveNode plugin does not handle all offered types of operations!");
			return nullptr;
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "cauchy_index", "sellmeier_index", "sellmeier2_index", "poly_index", "poly2_index" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::ReflectivePlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)