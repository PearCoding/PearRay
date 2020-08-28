#include "math/Microfacet.h"
#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Sampling.h"
#include "math/Scattering.h"
#include "math/Spherical.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

enum FresnelMode {
	FM_Dielectric,
	FM_Conductor
};

static struct {
	const char* Name;
	FresnelMode Mode;
} _fresnelMode[] = {
	{ "dielectric", FM_Dielectric },
	{ "conductor", FM_Conductor },
	{ "", FM_Dielectric }
};

enum DistributionMode {
	DM_Blinn,
	DM_Beckmann,
	DM_GGX
};

static struct {
	const char* Name;
	DistributionMode Mode;
} _distributionMode[] = {
	{ "blinn", DM_Blinn },
	{ "beckmann", DM_Beckmann },
	{ "ggx", DM_GGX },
	{ "", DM_Blinn }
};

enum GeometricMode {
	GM_Implicit,
	GM_Neumann,
	GM_Kelemen,
	GM_CookTorrance,
	GM_Schlick,
	GM_Walter
};

static struct {
	const char* Name;
	GeometricMode Mode;
} _geometryMode[] = {
	{ "implicit", GM_Implicit },
	{ "neumann", GM_Neumann },
	{ "kelemen", GM_Kelemen },
	{ "cooktorrance", GM_CookTorrance },
	{ "cook_torrance", GM_CookTorrance },
	{ "schlick", GM_Schlick },
	{ "walter", GM_Walter },
	{ "", GM_Implicit }
};

class MicrofacetMaterial : public IMaterial {
public:
	MicrofacetMaterial(uint32 id,
					   FresnelMode fresnelMode,
					   DistributionMode distributionMode,
					   GeometricMode geometryMode,
					   const std::shared_ptr<FloatSpectralNode>& alb,
					   const std::shared_ptr<FloatSpectralNode>& spec,
					   const std::shared_ptr<FloatSpectralNode>& ior,
					   const std::shared_ptr<FloatScalarNode>& roughnessX,
					   const std::shared_ptr<FloatScalarNode>& roughnessY,
					   const std::shared_ptr<FloatScalarNode>& conductorAbsorption)
		: IMaterial(id)
		, mFresnelMode(fresnelMode)
		, mDistributionMode(distributionMode)
		, mGeometricMode(geometryMode)
		, mAlbedo(alb)
		, mSpecularity(spec)
		, mIOR(ior)
		, mRoughnessX(roughnessX)
		, mRoughnessY(roughnessY)
		, mConductorAbsorption(conductorAbsorption)
	{
	}

	virtual ~MicrofacetMaterial() = default;

	SpectralBlob fresnelTerm(const MaterialSampleContext& ctx, const ShadingContext& sctx) const
	{
		// TODO: Add branching!
		const SpectralBlob ior = mIOR->eval(sctx);
		SpectralBlob out;

		switch (mFresnelMode) {
		default:
		case FM_Dielectric: {
			SpectralBlob n1 = SpectralBlob::Ones();
			SpectralBlob n2 = ior;
			if (ctx.NdotV() < 0)
				std::swap(n1, n2);
			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				out[i] = Fresnel::dielectric(std::abs(ctx.NdotV()), n1(i), n2(i));
		} break;
		case FM_Conductor: {
			const float a = mConductorAbsorption->eval(sctx);

			for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
				out[i] = Fresnel::conductor(std::abs(ctx.NdotV()), ior(i), a);
		} break;
		}
		return out;
	}

	float evalSpec(const MaterialEvalContext& ctx, const ShadingVector& H, const ShadingContext& sctx, float& pdf) const
	{
		if (H.cosTheta() <= PR_EPSILON
			|| ctx.NdotV() <= PR_EPSILON
			|| ctx.NdotL() <= PR_EPSILON) {
			pdf = 0;
			return 0.0f;
		}

		float m1 = mRoughnessX->eval(sctx);
		m1 *= m1;

		float G;
		switch (mGeometricMode) {
		case GM_Implicit:
			G = Microfacet::g_implicit(ctx.NdotV(), ctx.NdotL());
			break;
		case GM_Neumann:
			G = Microfacet::g_neumann(ctx.NdotV(), ctx.NdotL());
			break;
		default:
		case GM_CookTorrance:
			G = Microfacet::g_cook_torrance(ctx.NdotV(), ctx.NdotL(), H.cosTheta(), H.dot(ctx.V));
			break;
		case GM_Kelemen:
			G = Microfacet::g_kelemen(ctx.NdotV(), ctx.NdotL(), H.dot(ctx.V));
			break;
		case GM_Schlick:
			G = Microfacet::g_1_schlick(ctx.NdotV(), m1) * Microfacet::g_1_schlick(ctx.NdotL(), m1);
			break;
		case GM_Walter:
			G = Microfacet::g_1_walter(ctx.NdotV(), m1) * Microfacet::g_1_walter(ctx.NdotL(), m1);
			break;
		}

		float D;
		switch (mDistributionMode) {
		case DM_Blinn:
			D = Microfacet::ndf_blinn(H.cosTheta(), m1);
			break;
		case DM_Beckmann:
			D = Microfacet::ndf_beckmann(H.cosTheta(), m1);
			break;
		default:
		case DM_GGX:
			if (mRoughnessX == mRoughnessY) {
				D = Microfacet::ndf_ggx(H.cosTheta(), m1);
			} else {
				float m2 = mRoughnessY->eval(sctx);
				m2 *= m2;
				D = Microfacet::ndf_ggx(H, m1, m2);
			}
			break;
		}

		pdf = 0.25f * D / (H.cosTheta() * ctx.NdotL());
		return (0.25f * D * G) / (H.cosTheta() * ctx.NdotL());
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto& sctx = in.ShadingContext;
		SpectralBlob F	 = fresnelTerm(in.Context, sctx);

		SpectralBlob Diff = SpectralBlob::Zero();
		if (mFresnelMode != FM_Conductor)
			Diff = (F < 1).select(PR_INV_PI * mAlbedo->eval(sctx), 0.0f);

		SpectralBlob Spec = SpectralBlob::Zero();
		if (in.Context.NdotL() * in.Context.NdotV() > PR_EPSILON) {
			float pdf_s;
			Vector3f H = Scattering::halfway_reflection(in.Context.V, in.Context.L);
			Spec	   = mSpecularity->eval(sctx) * evalSpec(in.Context, H, sctx, pdf_s);
			out.PDF_S  = SpectralBlob(pdf_s);
		}

		out.PDF_S  = PR_INV_PI * (1 - F) + out.PDF_S * F;
		out.Weight = Diff * (1 - F) + Spec * F;
		out.Weight *= std::abs(in.Context.NdotL());
		out.Type = MST_DiffuseReflection;
	}

	void sampleDiffusePath(const MaterialSampleInput&, const ShadingContext& sctx, float u, float v, MaterialSampleOutput& out) const
	{
		out.L	   = Sampling::cos_hemi(u, v);
		out.PDF_S  = Sampling::cos_hemi_pdf(out.L(2));
		out.Weight = PR_INV_PI * mAlbedo->eval(sctx) * out.L[2];
		out.Type   = MST_DiffuseReflection;
	}

	void sampleSpecularPath(const MaterialSampleInput& in, const ShadingContext& sctx, float u, float v, MaterialSampleOutput& out) const
	{
		float m1 = mRoughnessX->eval(sctx);
		m1 *= m1;

		float pdf_s;
		Vector3f O;
		switch (mDistributionMode) {
		case DM_Blinn:
			O = Microfacet::sample_ndf_blinn(u, v, m1, pdf_s);
			break;
		default:
		case DM_Beckmann:
			O = Microfacet::sample_ndf_beckmann(u, v, m1, pdf_s);
			break;
		case DM_GGX:
			if (mRoughnessX == mRoughnessY) {
				O = Microfacet::sample_ndf_ggx(u, v, m1, pdf_s);
			} else {
				float m2 = mRoughnessY->eval(sctx);
				m2 *= m2;
				O = Microfacet::sample_ndf_ggx(u, v, m1, m2, pdf_s);
			}
			break;
		}

		out.L		= Scattering::reflect(in.Context.V, O);
		float NdotL = out.L[2];
		if (NdotL > PR_EPSILON)
			pdf_s = std::min(std::max(pdf_s / (-4 * NdotL * in.Context.NdotV()), 0.0f), 1.0f);
		else
			pdf_s = 0;

		out.Type  = MST_SpecularReflection;
		out.PDF_S = SpectralBlob(pdf_s);

		float _ignore;
		out.Weight = mSpecularity->eval(sctx) * evalSpec(in.Context.expand(out.L), O, sctx, _ignore) * NdotL;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto& sctx = in.ShadingContext;
		SpectralBlob F	 = fresnelTerm(in.Context, sctx);
		float decision_f = F[0];

		if (in.RND[0] <= decision_f) {
			sampleSpecularPath(in, sctx, in.RND[0] / decision_f, in.RND[1], out);
			out.PDF_S *= decision_f;
			out.Type = MST_SpecularReflection;
		} else if (mFresnelMode == FM_Conductor) {
			// Absorb
			out.Weight = 0.0f;
			out.PDF_S  = 0.0f;
		} else {
			sampleDiffusePath(in, sctx, (in.RND[0] - decision_f) / (1 - decision_f), in.RND[1], out);
			out.PDF_S *= 1.0f - decision_f;
			out.Type = MST_DiffuseReflection;
		}

		// No translucent
		if (out.L[2] < PR_EPSILON)
			out.PDF_S = 0;
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		std::string FT = "UNKNOWN";
		std::string GT = "UNKNOWN";
		std::string DT = "UNKNOWN";
		for (size_t i = 0; _fresnelMode[i].Name; ++i) {
			if (_fresnelMode[i].Mode == mFresnelMode) {
				FT = _fresnelMode[i].Name;
				break;
			}
		}
		for (size_t i = 0; _geometryMode[i].Name; ++i) {
			if (_geometryMode[i].Mode == mGeometricMode) {
				GT = _geometryMode[i].Name;
				break;
			}
		}
		for (size_t i = 0; _distributionMode[i].Name; ++i) {
			if (_distributionMode[i].Mode == mDistributionMode) {
				DT = _distributionMode[i].Name;
				break;
			}
		}

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <MicrofacetMaterial>:" << std::endl
			   << "    FresnelMode:         " << FT << std::endl
			   << "    GeometricMode:       " << GT << std::endl
			   << "    DistributionMode:    " << DT << std::endl
			   << "    Albedo:              " << (mAlbedo ? mAlbedo->dumpInformation() : "NONE") << std::endl
			   << "    Specularity:         " << (mSpecularity ? mSpecularity->dumpInformation() : "NONE") << std::endl
			   << "    IOR:                 " << (mIOR ? mIOR->dumpInformation() : "NONE") << std::endl
			   << "    RoughnessX:          " << (mRoughnessX ? mRoughnessX->dumpInformation() : "NONE") << std::endl
			   << "    RoughnessY:          " << (mRoughnessY ? mRoughnessY->dumpInformation() : "NONE") << std::endl
			   << "    ConductorAbsorption: " << (mConductorAbsorption ? mConductorAbsorption->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

private:
	FresnelMode mFresnelMode;
	DistributionMode mDistributionMode;
	GeometricMode mGeometricMode;
	std::shared_ptr<FloatSpectralNode> mAlbedo;
	std::shared_ptr<FloatSpectralNode> mSpecularity;
	std::shared_ptr<FloatSpectralNode> mIOR;
	std::shared_ptr<FloatScalarNode> mRoughnessX;
	std::shared_ptr<FloatScalarNode> mRoughnessY;
	std::shared_ptr<FloatScalarNode> mConductorAbsorption;
}; // namespace PR

class MicrofacetMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string fMStr = params.getString("fresnel_mode", "");
		const std::string gMStr = params.getString("geometric_mode", "");
		const std::string dMStr = params.getString("distribution_mode", "");

		std::shared_ptr<FloatScalarNode> roughnessX;
		std::shared_ptr<FloatScalarNode> roughnessY;
		const auto roughnessP = params.getParameter("roughness");

		if (roughnessP.isValid()) {
			roughnessX = ctx.Env->lookupScalarNode(roughnessP, 0.5f);
			roughnessY = roughnessX;
		} else {
			roughnessX = ctx.Env->lookupScalarNode(params.getParameter("roughness_x"), 0.5f);
			roughnessY = ctx.Env->lookupScalarNode(params.getParameter("roughness_y"), 0.5f);
		}

		FresnelMode fm		= FM_Dielectric;
		GeometricMode gm	= GM_Implicit;
		DistributionMode dm = DM_GGX;
		for (size_t i = 0; _fresnelMode[i].Name; ++i) {
			if (_fresnelMode[i].Name == fMStr) {
				fm = _fresnelMode[i].Mode;
				break;
			}
		}
		for (size_t i = 0; _geometryMode[i].Name; ++i) {
			if (_geometryMode[i].Name == gMStr) {
				gm = _geometryMode[i].Mode;
				break;
			}
		}
		for (size_t i = 0; _distributionMode[i].Name; ++i) {
			if (_distributionMode[i].Name == dMStr) {
				dm = _distributionMode[i].Mode;
				break;
			}
		}

		return std::make_shared<MicrofacetMaterial>(id,
													fm, dm, gm,
													ctx.Env->lookupSpectralNode(params.getParameter("albedo"), 1),
													ctx.Env->lookupSpectralNode(params.getParameter("specularity"), 1),
													ctx.Env->lookupSpectralNode(params.getParameter("index"), 1.55f),
													roughnessX, roughnessY,
													ctx.Env->lookupScalarNode(params.getParameter("conductor_absorption"), 0.5f));
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "microfacet", "cooktorrance", "cook_torrance" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::MicrofacetMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)