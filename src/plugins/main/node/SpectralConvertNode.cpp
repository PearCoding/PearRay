#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/INodePlugin.h"
#include "spectral/CIE.h"
#include "spectral/RGBConverter.h"

namespace PR {

static inline SpectralBlob conv_x(const SpectralBlob& wvl)
{
	SpectralBlob blob;
	PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
	for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
		blob[i] = CIE::eval_x(wvl[i]);
	return blob;
}

static inline SpectralBlob conv_y(const SpectralBlob& wvl)
{
	SpectralBlob blob;
	PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
	for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
		blob[i] = CIE::eval_y(wvl[i]);
	return blob;
}

static inline SpectralBlob conv_z(const SpectralBlob& wvl)
{
	SpectralBlob blob;
	PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
	for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
		blob[i] = CIE::eval_z(wvl[i]);
	return blob;
}

static inline void rgb(const SpectralBlob& wvl, SpectralBlob& r, SpectralBlob& g, SpectralBlob& b)
{
	const auto X = conv_x(wvl);
	const auto Y = conv_y(wvl);
	const auto Z = conv_z(wvl);

	PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
	for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
		RGBConverter::fromXYZ(X[i], Y[i], Z[i], r[i], g[i], b[i]);
}

static inline void hsv(const SpectralBlob& w, const SpectralBlob& wvl, SpectralBlob& hue, SpectralBlob& saturation, SpectralBlob& value)
{
	SpectralBlob r, g, b;
	rgb(wvl, r, g, b);

	r *= w;
	g *= w;
	b *= w;

	const SpectralBlob rgbMin = r.cwiseMin(g).cwiseMin(b);
	const SpectralBlob rgbMax = r.cwiseMax(g).cwiseMax(b);
	const SpectralBlob diff	  = rgbMax - rgbMin;

	value	   = rgbMax;
	saturation = diff / rgbMax;

	const SpectralBlob diffR = (((rgbMax - r) / 6) + (diff / 2)) / diff;
	const SpectralBlob diffG = (((rgbMax - g) / 6) + (diff / 2)) / diff;
	const SpectralBlob diffB = (((rgbMax - b) / 6) + (diff / 2)) / diff;

	hue		   = (r == rgbMax).select(diffB - diffG, (g == rgbMax).select((1.0f / 3) + diffR - diffB, (2.0f / 3) + diffG - diffR));
	hue		   = (hue < 0).select(hue + 1, hue);
	hue		   = (hue > 1).select(hue - 1, hue);
	hue		   = (diff == 0).select(SpectralBlob::Zero(), hue);
	saturation = (diff == 0).select(SpectralBlob::Zero(), saturation);
}

#define _OP(Prefix, F)                                                                 \
	class Prefix##SpectralConvert : public FloatSpectralNode {                         \
	public:                                                                            \
		explicit Prefix##SpectralConvert(const std::shared_ptr<FloatSpectralNode>& op) \
			: mOperand(op)                                                             \
		{                                                                              \
		}                                                                              \
		SpectralBlob eval(const ShadingContext& ctx) const override                    \
		{                                                                              \
			return F(mOperand->eval(ctx), ctx.WavelengthNM);                           \
		}                                                                              \
		Vector2i queryRecommendedSize() const override                                 \
		{                                                                              \
			return mOperand->queryRecommendedSize();                                   \
		}                                                                              \
		std::string dumpInformation() const override                                   \
		{                                                                              \
			std::stringstream sstream;                                                 \
			sstream << PR_STRINGIFY(F) " (" << mOperand->dumpInformation() << ")";     \
			return sstream.str();                                                      \
		}                                                                              \
                                                                                       \
	private:                                                                           \
		const std::shared_ptr<FloatSpectralNode> mOperand;                             \
	}

inline static SpectralBlob sciex(const SpectralBlob& v, const SpectralBlob& wvl)
{
	return v * conv_x(wvl);
}
inline static SpectralBlob sciey(const SpectralBlob& v, const SpectralBlob& wvl)
{
	return v * conv_y(wvl);
}
inline static SpectralBlob sciez(const SpectralBlob& v, const SpectralBlob& wvl)
{
	return v * conv_z(wvl);
}

inline static SpectralBlob sred(const SpectralBlob& v, const SpectralBlob& wvl)
{
	SpectralBlob r, g, b;
	rgb(wvl, r, g, b);
	return v * r;
}

inline static SpectralBlob sgreen(const SpectralBlob& v, const SpectralBlob& wvl)
{
	SpectralBlob r, g, b;
	rgb(wvl, r, g, b);
	return v * g;
}

inline static SpectralBlob sblue(const SpectralBlob& v, const SpectralBlob& wvl)
{
	SpectralBlob r, g, b;
	rgb(wvl, r, g, b);
	return v * b;
}

inline static SpectralBlob shue(const SpectralBlob& v, const SpectralBlob& wvl)
{
	SpectralBlob hue, saturation, value;
	hsv(v, wvl, hue, saturation, value);
	return hue;
}

inline static SpectralBlob ssaturation(const SpectralBlob& v, const SpectralBlob& wvl)
{
	SpectralBlob hue, saturation, value;
	hsv(v, wvl, hue, saturation, value);
	return saturation;
}

inline static SpectralBlob svalue(const SpectralBlob& v, const SpectralBlob& wvl)
{
	SpectralBlob hue, saturation, value;
	hsv(v, wvl, hue, saturation, value);
	return value;
}

_OP(CIEX, sciex);
_OP(CIEY, sciey);
_OP(CIEZ, sciez);
_OP(Red, sred);
_OP(Green, sgreen);
_OP(Blue, sblue);
_OP(Hue, shue);
_OP(Saturation, ssaturation);
_OP(Value, svalue);

class SpectralConvertPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string& type_name, const SceneLoadContext& ctx) override
	{
		if (type_name == "sciex") {
			return std::make_shared<CIEXSpectralConvert>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "sciey") {
			return std::make_shared<CIEYSpectralConvert>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "sciez") {
			return std::make_shared<CIEZSpectralConvert>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "sred") {
			return std::make_shared<RedSpectralConvert>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "sgreen") {
			return std::make_shared<GreenSpectralConvert>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "sblue") {
			return std::make_shared<BlueSpectralConvert>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "shue") {
			return std::make_shared<HueSpectralConvert>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "svalue") {
			return std::make_shared<ValueSpectralConvert>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else if (type_name == "ssaturation") {
			return std::make_shared<SaturationSpectralConvert>(ctx.lookupSpectralNode(ctx.parameters().getParameter(0)));
		} else {
			PR_ASSERT(false, "SpectralConvertPlugin plugin does not handle all offered types of operations!");
			return nullptr;
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "sciex", "sciey", "sciez",
													  "sred", "sgreen", "sblue",
													  "shue", "svalue", "ssaturation" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Spectral Convert Node", "A node to convert between different spaces. Keep in mind that the raytracer is a full spectral one!")
			.Identifiers(getNames())
			.Inputs()
			.SpectralNode("op", "Operand", 0.0f)
			.Specification()
			.get();
	}

	
};
} // namespace PR

PR_PLUGIN_INIT(PR::SpectralConvertPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)