#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "math/Scattering.h"
#include "shader/ConstNode.h"
#include "shader/INodePlugin.h"

#include <array>

namespace PR {

inline float teval(const ShadingContext&, float a) { return a; }
inline SpectralBlob teval(const ShadingContext& ctx, const std::shared_ptr<FloatSpectralNode>& a) { return a->eval(ctx); }

inline std::string tdump(float a) { return std::to_string(a); }
inline std::string tdump(const std::shared_ptr<FloatSpectralNode>& a) { return a->dumpInformation(); }

template <typename T, size_t N>
class CauchyIndexNode : public FloatSpectralNode {
public:
	explicit CauchyIndexNode(const std::array<T, N>& cs)
		: FloatSpectralNode(NodeFlag::SpectralVarying)
		, mCs(cs)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		using RetType = decltype(teval(ctx, mCs[0]));
		std::array<RetType, N> cs;
		PR_OPT_LOOP
		for (size_t i = 0; i < N; ++i)
			cs[i] = teval(ctx, mCs[i]);
		return Scattering::cauchy<SpectralBlob, RetType>(ctx.WavelengthNM, cs.data(), N);
	}

	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "CauchyIndex (";
		for (size_t i = 0; i < N - 1; ++i)
			sstream << tdump(mCs[i]) << ", ";
		sstream << tdump(mCs[N - 1]) << ")";
		return sstream.str();
	}

private:
	const std::array<T, N> mCs;
};

template <typename T, size_t N>
inline std::shared_ptr<CauchyIndexNode<T, N>> create_cauchy_index(const std::array<T, N>& cs)
{
	return std::make_shared<CauchyIndexNode<T, N>>(cs);
}

template <typename T, size_t N>
class SellmeierSqrIndexNode : public FloatSpectralNode {
public:
	explicit SellmeierSqrIndexNode(const std::array<T, N>& bs, const std::array<T, N>& cs)
		: FloatSpectralNode(NodeFlag::SpectralVarying)
		, mBs(bs)
		, mCs(cs)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		using RetType = decltype(teval(ctx, mBs[0]));
		std::array<RetType, N> bs;
		std::array<RetType, N> cs;
		PR_OPT_LOOP
		for (size_t i = 0; i < N; ++i) {
			bs[i] = teval(ctx, mBs[i]);
			cs[i] = teval(ctx, mCs[i]);
		}
		return Scattering::sellmeier2<SpectralBlob, RetType>(ctx.WavelengthNM, bs.data(), cs.data(), N);
	}

	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "SquaredSellmeierIndex (";
		for (size_t i = 0; i < N; ++i)
			sstream << "[" << tdump(mBs[i]) << "," << tdump(mCs[i]) << "]";
		sstream << ")";
		return sstream.str();
	}

private:
	const std::array<T, N> mBs;
	const std::array<T, N> mCs;
};

template <typename T, size_t N>
inline std::shared_ptr<SellmeierSqrIndexNode<T, N>> create_squared_sellmeier_index(const std::array<T, N>& bs, const std::array<T, N>& cs)
{
	return std::make_shared<SellmeierSqrIndexNode<T, N>>(bs, cs);
}

template <typename T, size_t N>
class SellmeierIndexNode : public FloatSpectralNode {
public:
	explicit SellmeierIndexNode(const std::array<T, N>& bs, const std::array<T, N>& cs)
		: FloatSpectralNode(NodeFlag::SpectralVarying)
		, mBs(bs)
		, mCs(cs)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		using RetType = decltype(teval(ctx, mBs[0]));
		std::array<RetType, N> bs;
		std::array<RetType, N> cs;
		PR_OPT_LOOP
		for (size_t i = 0; i < N; ++i) {
			bs[i] = teval(ctx, mBs[i]);
			cs[i] = teval(ctx, mCs[i]);
		}
		return Scattering::sellmeier2<SpectralBlob, RetType>(ctx.WavelengthNM, bs.data(), cs.data(), N).cwiseSqrt();
	}

	Vector2i queryRecommendedSize() const override { return Vector2i(1, 1); }
	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "SellmeierIndex (";
		for (size_t i = 0; i < N; ++i)
			sstream << "[" << tdump(mBs[i]) << "," << tdump(mCs[i]) << "]";
		sstream << ")";
		return sstream.str();
	}

private:
	const std::array<T, N> mBs;
	const std::array<T, N> mCs;
};

template <typename T, size_t N>
inline std::shared_ptr<SellmeierIndexNode<T, N>> create_sellmeier_index(const std::array<T, N>& bs, const std::array<T, N>& cs)
{
	return std::make_shared<SellmeierIndexNode<T, N>>(bs, cs);
}

template <typename T>
class PolySqrIndexNode : public FloatSpectralNode {
public:
	explicit PolySqrIndexNode(const T& a, const T& b1, const T& b2, const T& c1, const T& c2)
		: FloatSpectralNode(NodeFlag::SpectralVarying)
		, mA(a)
		, mB1(b1)
		, mB2(b2)
		, mC1(c1)
		, mC2(c2)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		return Scattering::poly2(ctx.WavelengthNM, teval(ctx, mA),
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
		: FloatSpectralNode(NodeFlag::SpectralVarying)
		, mA(a)
		, mB1(b1)
		, mB2(b2)
		, mC1(c1)
		, mC2(c2)
	{
	}

	SpectralBlob eval(const ShadingContext& ctx) const override
	{
		return Scattering::poly2(ctx.WavelengthNM, teval(ctx, mA),
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

inline std::shared_ptr<ConstSpectralNode> create_const_index(float a)
{
	return std::make_shared<ConstSpectralNode>(SpectralBlob(a));
}

// https://refractiveindex.info/?shelf=glass&book=BK7&page=SCHOTT
static const auto BK7 = create_sellmeier_index<float, 3>({ 1.03961212f, 0.231792344f, 1.01046945f }, { 0.00600069867f, 0.0200179144f, 103.560653f });
// https://refractiveindex.info/?shelf=main&book=H2O&page=Daimon-20.0C
static const auto H2O = create_sellmeier_index<float, 4>({ 5.684027565e-1f, 1.726177391e-1f, 2.086189578e-2f, 1.130748688e-1f },
														 { 5.101829712e-3f, 1.821153936e-2f, 2.620722293e-2f, 1.069792721e1f });
// https://refractiveindex.info/?shelf=main&book=C&page=Peter
static const auto DIA = create_sellmeier_index<float, 2>({ 0.3306f, 4.3356f }, { 0.030625f, 0.011236f });

static const auto VACUUM = create_const_index(1.0f);
static const auto AIR	 = create_const_index(1.000277f);
static struct {
	const char* Name;
	std::shared_ptr<FloatSpectralNode> Node;
} _lookups[] = {
	{ "bk7", BK7 },
	{ "glass", BK7 },
	{ "h2o", H2O },
	{ "water", H2O },
	{ "diamond", DIA },
	{ "vacuum", VACUUM },
	{ "none", VACUUM },
	{ "air", AIR },
	{ nullptr, nullptr }
};

using ParamNode = std::shared_ptr<FloatSpectralNode>;

template <size_t N>
std::shared_ptr<FloatSpectralNode> createCauchyIndex(const SceneLoadContext& ctx, bool allNumber)
{
	if (allNumber) {
		std::array<float, N> cs;
		for (size_t i = 0; i < N; ++i)
			cs[i] = ctx.parameters().getParameter(i).getNumber(0.0f);
		return create_cauchy_index(cs);
	} else {
		std::array<ParamNode, N> cs;
		for (size_t i = 0; i < N; ++i)
			cs[i] = ctx.lookupSpectralNode(ctx.parameters().getParameter(i));
		return create_cauchy_index(cs);
	}
}

template <bool isSquared, size_t N>
std::shared_ptr<FloatSpectralNode> createSellmeierIndex(const SceneLoadContext& ctx, bool allNumber)
{
	if (allNumber) {
		std::array<float, N> bs;
		std::array<float, N> cs;
		for (size_t i = 0; i < N; ++i)
			bs[i] = ctx.parameters().getParameter(i).getNumber(0.0f);
		for (size_t i = 0; i < N; ++i)
			cs[i] = ctx.parameters().getParameter(i + N).getNumber(0.0f);
		if constexpr (isSquared)
			return create_squared_sellmeier_index(bs, cs);
		else
			return create_sellmeier_index(bs, cs);
	} else {
		std::array<ParamNode, N> bs;
		std::array<ParamNode, N> cs;
		for (size_t i = 0; i < N; ++i)
			bs[i] = ctx.lookupSpectralNode(ctx.parameters().getParameter(i));
		for (size_t i = 0; i < N; ++i)
			cs[i] = ctx.lookupSpectralNode(ctx.parameters().getParameter(i + N));
		if constexpr (isSquared)
			return create_squared_sellmeier_index(bs, cs);
		else
			return create_sellmeier_index(bs, cs);
	}
}

class ReflectivePlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string& type_name, const SceneLoadContext& ctx) override
	{
		bool allNumber = true;
		for (const auto& param : ctx.parameters().positionalParameters()) {
			if (!param.canBeNumber()) {
				allNumber = false;
				break;
			}
		}

		if (type_name == "cauchy_index") {
			size_t N = ctx.parameters().positionalParameters().size(); // Glue between runtime and template
			switch (N) {
			case 0:
			case 1:
				return createCauchyIndex<1>(ctx, allNumber);
			case 2:
				return createCauchyIndex<2>(ctx, allNumber);
			case 3:
				return createCauchyIndex<3>(ctx, allNumber);
			case 4:
				return createCauchyIndex<4>(ctx, allNumber);
			case 5:
				return createCauchyIndex<5>(ctx, allNumber);
			default:
			case 6:
				return createCauchyIndex<6>(ctx, allNumber);
			}
		} else if (type_name == "sellmeier_index") {
			size_t N = ctx.parameters().positionalParameters().size() / 2; // Glue between runtime and template
			switch (N) {
			case 0:
			case 1:
				return createSellmeierIndex<false, 1>(ctx, allNumber);
			case 2:
				return createSellmeierIndex<false, 2>(ctx, allNumber);
			case 3:
				return createSellmeierIndex<false, 3>(ctx, allNumber);
			case 4:
				return createSellmeierIndex<false, 4>(ctx, allNumber);
			case 5:
				return createSellmeierIndex<false, 5>(ctx, allNumber);
			default:
			case 6:
				return createSellmeierIndex<false, 6>(ctx, allNumber);
			}
		} else if (type_name == "sellmeier2_index") {
			size_t N = ctx.parameters().positionalParameters().size() / 2; // Glue between runtime and template
			switch (N) {
			case 0:
			case 1:
				return createSellmeierIndex<true, 1>(ctx, allNumber);
			case 2:
				return createSellmeierIndex<true, 2>(ctx, allNumber);
			case 3:
				return createSellmeierIndex<true, 3>(ctx, allNumber);
			case 4:
				return createSellmeierIndex<true, 4>(ctx, allNumber);
			case 5:
				return createSellmeierIndex<true, 5>(ctx, allNumber);
			default:
			case 6:
				return createSellmeierIndex<true, 6>(ctx, allNumber);
			}
		} else if (type_name == "poly_index") {
			if (allNumber)
				return std::make_shared<PolyIndexNode<float>>(ctx.parameters().getParameter(0).getNumber(0.0f),
															  ctx.parameters().getParameter(1).getNumber(0.0f),
															  ctx.parameters().getParameter(2).getNumber(0.0f),
															  ctx.parameters().getParameter(3).getNumber(0.0f),
															  ctx.parameters().getParameter(4).getNumber(0.0f));
			else
				return std::make_shared<PolyIndexNode<ParamNode>>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
																  ctx.lookupSpectralNode(ctx.parameters().getParameter(1)),
																  ctx.lookupSpectralNode(ctx.parameters().getParameter(2)),
																  ctx.lookupSpectralNode(ctx.parameters().getParameter(3)),
																  ctx.lookupSpectralNode(ctx.parameters().getParameter(4)));
		} else if (type_name == "poly2_index") {
			if (allNumber)
				return std::make_shared<PolySqrIndexNode<float>>(ctx.parameters().getParameter(0).getNumber(0.0f),
																 ctx.parameters().getParameter(1).getNumber(0.0f),
																 ctx.parameters().getParameter(2).getNumber(0.0f),
																 ctx.parameters().getParameter(3).getNumber(0.0f),
																 ctx.parameters().getParameter(4).getNumber(0.0f));
			else
				return std::make_shared<PolySqrIndexNode<ParamNode>>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)),
																	 ctx.lookupSpectralNode(ctx.parameters().getParameter(1)),
																	 ctx.lookupSpectralNode(ctx.parameters().getParameter(2)),
																	 ctx.lookupSpectralNode(ctx.parameters().getParameter(3)),
																	 ctx.lookupSpectralNode(ctx.parameters().getParameter(4)));
		} else if (type_name == "lookup_index") {
			auto name = ctx.parameters().getParameter(0).getString("bk7");
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);

			for (int i = 0; _lookups[i].Name; ++i) {
				if (_lookups[i].Name == name)
					return _lookups[i].Node;
			}

			PR_LOG(L_ERROR) << "Unknown lookup name " << name << std::endl;
			return nullptr;
		} else {
			PR_ASSERT(false, "ReflectiveNode plugin does not handle all offered types of operations!");
			return nullptr;
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "cauchy_index", "sellmeier_index", "sellmeier2_index", "poly_index", "poly2_index", "lookup_index" });
		return names;
	}

	PluginSpecification specification(const std::string& type_name) const override
	{
		if (type_name == "cauchy_index") {
			return PluginSpecificationBuilder("Cauchy Index Node", "A spectral value based on the cauchy index")
				.Identifier(type_name)
				.Inputs()
				.Number("0", "First entry", 0.0f)
				.Number("1", "Second entry", 0.0f)
				.Number("2", "Third entry", 0.0f, true)
				.Number("3", "Fourth entry", 0.0f, true)
				.Number("4", "Fifth entry", 0.0f, true)
				.Number("5", "Sixth entry", 0.0f, true)
				.Number("6", "Seventh entry", 0.0f, true)
				.Specification()
				.get();
		} else if (type_name == "sellmeier_index") {
			return PluginSpecificationBuilder("Sellmeier Index Node", "A spectral value based on the sellmeier index")
				.Identifier(type_name)
				.Inputs()
				.Number("0", "First entry", 0.0f)
				.Number("1", "Second entry", 0.0f)
				.Number("2", "Third entry", 0.0f, true)
				.Number("3", "Fourth entry", 0.0f, true)
				.Number("4", "Fifth entry", 0.0f, true)
				.Number("5", "Sixth entry", 0.0f, true)
				.Number("6", "Seventh entry", 0.0f, true)
				.Specification()
				.get();
		} else if (type_name == "sellmeier2_index") {
			return PluginSpecificationBuilder("Squared Sellmeier Index Node", "A spectral value based on the squard sellmeier index")
				.Identifier(type_name)
				.Inputs()
				.Number("0", "First entry", 0.0f)
				.Number("1", "Second entry", 0.0f)
				.Number("2", "Third entry", 0.0f, true)
				.Number("3", "Fourth entry", 0.0f, true)
				.Number("4", "Fifth entry", 0.0f, true)
				.Number("5", "Sixth entry", 0.0f, true)
				.Number("6", "Seventh entry", 0.0f, true)
				.Specification()
				.get();
		} else if (type_name == "poly_index") {
			return PluginSpecificationBuilder("Polynomial Node", "A spectral value based on a polynomial")
				.Identifier(type_name)
				.Inputs()
				.Number("0", "First entry", 0.0f)
				.Number("1", "Second entry", 0.0f)
				.Number("2", "Third entry", 0.0f, true)
				.Number("3", "Fourth entry", 0.0f, true)
				.Number("4", "Fifth entry", 0.0f, true)
				.Specification()
				.get();
		} else if (type_name == "poly2_index") {
			return PluginSpecificationBuilder("Squared Polynomial Node", "A spectral value based on a squard polynomial")
				.Identifier(type_name)
				.Inputs()
				.Number("0", "First entry", 0.0f)
				.Number("1", "Second entry", 0.0f)
				.Number("2", "Third entry", 0.0f, true)
				.Number("3", "Fourth entry", 0.0f, true)
				.Number("4", "Fifth entry", 0.0f, true)
				.Specification()
				.get();
		} else if (type_name == "lookup_index") {
			std::vector<std::string> lookups;
			for (int i = 0; _lookups[i].Name; ++i)
				lookups.emplace_back(_lookups[i].Name);
			return PluginSpecificationBuilder("Lookup Node", "A spectral value based on predefined distributions")
				.Identifier(type_name)
				.Inputs()
				.Option("name", "Name of the given lookup distribution", "bk7", lookups)
				.Specification()
				.get();
		} else {
			PR_ASSERT(false, "ReflectiveNode plugin does not handle all offered types of operations!");
			return PluginSpecification("INVALID", "INVALID");
		}
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::ReflectivePlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)