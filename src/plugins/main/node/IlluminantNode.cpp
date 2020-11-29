#include "Environment.h"
#include "Logger.h"
#include "SceneLoadContext.h"
#include "shader/ConstNode.h"
#include "shader/EquidistantSpectrumNode.h"
#include "shader/INodePlugin.h"

namespace PR {
#define _ILLUMINANT(Prefix)                                          \
	class Prefix##Illuminant : public EquidistantSpectrumViewNode {  \
	public:                                                          \
		explicit Prefix##Illuminant();                               \
		SpectralBlob eval(const ShadingContext& ctx) const override; \
		std::string dumpInformation() const override;                \
	};

_ILLUMINANT(D65)
_ILLUMINANT(D50)
_ILLUMINANT(D55)
_ILLUMINANT(D75)
_ILLUMINANT(A)
_ILLUMINANT(C)
_ILLUMINANT(F1)
_ILLUMINANT(F2)
_ILLUMINANT(F3)
_ILLUMINANT(F4)
_ILLUMINANT(F5)
_ILLUMINANT(F6)
_ILLUMINANT(F7)
_ILLUMINANT(F8)
_ILLUMINANT(F9)
_ILLUMINANT(F10)
_ILLUMINANT(F11)
_ILLUMINANT(F12)

#undef _ILLUMINANT

#include "IlluminantData.inl"

#define _ILLUMINANT_DEC(Prefix, Samples, Start, End)                                               \
	Prefix##Illuminant::Prefix##Illuminant()                                                       \
		: EquidistantSpectrumViewNode(EquidistantSpectrumView(Prefix##_data, Samples, Start, End)) \
	{                                                                                              \
	}                                                                                              \
	SpectralBlob Prefix##Illuminant::eval(const ShadingContext& ctx) const                         \
	{                                                                                              \
		return EquidistantSpectrumViewNode::eval(ctx);                                             \
	}                                                                                              \
	std::string Prefix##Illuminant::dumpInformation() const                                        \
	{                                                                                              \
		std::stringstream sstream;                                                                 \
		sstream << "Illuminant " PR_STRINGIFY(Prefix);                                             \
		return sstream.str();                                                                      \
	}

#define _ILLUMINANT_DEC_C(Prefix) _ILLUMINANT_DEC(Prefix, CIE_SampleCount, CIE_WavelengthStart, CIE_WavelengthEnd)
#define _ILLUMINANT_DEC_F(Prefix) _ILLUMINANT_DEC(Prefix, CIE_F_SampleCount, CIE_F_WavelengthStart, CIE_F_WavelengthEnd)

_ILLUMINANT_DEC_C(D65)
_ILLUMINANT_DEC_C(D50)
_ILLUMINANT_DEC_C(D55)
_ILLUMINANT_DEC_C(D75)
_ILLUMINANT_DEC_C(A)
_ILLUMINANT_DEC_C(C)

_ILLUMINANT_DEC_F(F1)
_ILLUMINANT_DEC_F(F2)
_ILLUMINANT_DEC_F(F3)
_ILLUMINANT_DEC_F(F4)
_ILLUMINANT_DEC_F(F5)
_ILLUMINANT_DEC_F(F6)
_ILLUMINANT_DEC_F(F7)
_ILLUMINANT_DEC_F(F8)
_ILLUMINANT_DEC_F(F9)
_ILLUMINANT_DEC_F(F10)
_ILLUMINANT_DEC_F(F11)
_ILLUMINANT_DEC_F(F12)

class IlluminantPlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string&, const SceneLoadContext& ctx) override
	{
		std::string illum = ctx.parameters().getString("spectrum", "");
		if (illum.empty())
			illum = ctx.parameters().getString(0, "D65");

		std::transform(illum.begin(), illum.end(), illum.begin(), [](char c) { return std::tolower(c); });

		if (illum == "d65")
			return std::make_shared<D65Illuminant>();
		else if (illum == "d50")
			return std::make_shared<D50Illuminant>();
		else if (illum == "d55")
			return std::make_shared<D55Illuminant>();
		else if (illum == "d75")
			return std::make_shared<D75Illuminant>();
		else if (illum == "a")
			return std::make_shared<AIlluminant>();
		else if (illum == "c")
			return std::make_shared<CIlluminant>();
		else if (illum == "e")
			return std::make_shared<ConstSpectralNode>(SpectralBlob::Ones());
		else if (illum == "f1")
			return std::make_shared<F1Illuminant>();
		else if (illum == "f2")
			return std::make_shared<F2Illuminant>();
		else if (illum == "f3")
			return std::make_shared<F3Illuminant>();
		else if (illum == "f4")
			return std::make_shared<F4Illuminant>();
		else if (illum == "f5")
			return std::make_shared<F5Illuminant>();
		else if (illum == "f6")
			return std::make_shared<F6Illuminant>();
		else if (illum == "f7")
			return std::make_shared<F7Illuminant>();
		else if (illum == "f8")
			return std::make_shared<F8Illuminant>();
		else if (illum == "f9")
			return std::make_shared<F9Illuminant>();
		else if (illum == "f10")
			return std::make_shared<F10Illuminant>();
		else if (illum == "f11")
			return std::make_shared<F11Illuminant>();
		else if (illum == "f12")
			return std::make_shared<F12Illuminant>();
		else {
			PR_LOG(L_ERROR) << "Unknown illuminant spectrum " << illum << std::endl;
			return nullptr;
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "illuminant" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::IlluminantPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)