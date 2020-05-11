#include "math/Microfacet.h"
#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Projection.h"
#include "math/Reflection.h"
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
					   const std::shared_ptr<FloatSpectralShadingSocket>& alb,
					   const std::shared_ptr<FloatSpectralShadingSocket>& spec,
					   const std::shared_ptr<FloatSpectralShadingSocket>& ior,
					   const std::shared_ptr<FloatScalarShadingSocket>& roughnessX,
					   const std::shared_ptr<FloatScalarShadingSocket>& roughnessY,
					   const std::shared_ptr<FloatScalarShadingSocket>& conductorAbsorption)
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

	void startGroup(size_t, const RenderTileSession&) override
	{
	}

	void endGroup() override
	{
	}

	float fresnelTerm(const ShadingPoint& point) const
	{
		// TODO: Add branching!
		const float ior = mIOR->eval(point).maxCoeff();

		switch (mFresnelMode) {
		default:
		case FM_Dielectric: {
			float n1 = 1;
			float n2 = ior;
			if (point.Flags & SPF_Inside)
				std::swap(n1, n2);
			return Fresnel::dielectric(-point.NdotV, n1, n2);
		}
		case FM_Conductor: {
			float a = mConductorAbsorption->eval(point);

			return Fresnel::conductor(-point.NdotV, ior, a);
		};
		}
	}

	float evalSpec(const ShadingPoint& point, const Vector3f& L, float NdotL, float& pdf) const
	{
		const Eigen::Vector3f H = Reflection::halfway(point.Ray.Direction, L);
		const float NdotH		= point.N.dot(H);
		const float NdotV		= -point.NdotV;

		if (NdotH <= PR_EPSILON
			|| NdotV <= PR_EPSILON
			|| NdotL <= PR_EPSILON) {
			pdf = 0;
			return 0.0f;
		}

		float m1 = mRoughnessX->eval(point);
		m1 *= m1;

		float G;
		switch (mGeometricMode) {
		case GM_Implicit:
			G = Microfacet::g_implicit(NdotV, NdotL);
			break;
		case GM_Neumann:
			G = Microfacet::g_neumann(NdotV, NdotL);
			break;
		default:
		case GM_CookTorrance:
			G = Microfacet::g_cook_torrance(NdotV, NdotL, NdotH, -point.Ray.Direction.dot(H));
			break;
		case GM_Kelemen:
			G = Microfacet::g_kelemen(NdotV, NdotL, -point.Ray.Direction.dot(H));
			break;
		case GM_Schlick:
			G = Microfacet::g_1_schlick(NdotV, m1) * Microfacet::g_1_schlick(NdotL, m1);
			break;
		case GM_Walter:
			G = Microfacet::g_1_walter(NdotV, m1) * Microfacet::g_1_walter(NdotL, m1);
			break;
		}

		float D;
		switch (mDistributionMode) {
		case DM_Blinn:
			D = Microfacet::ndf_blinn(NdotH, m1);
			break;
		case DM_Beckmann:
			D = Microfacet::ndf_beckmann(NdotH, m1);
			break;
		default:
		case DM_GGX:
			if (mRoughnessX == mRoughnessY) {
				D = Microfacet::ndf_ggx(NdotH, m1);
			} else {
				float m2 = mRoughnessY->eval(point);
				m2 *= m2;

				const float XdotH = std::abs(point.Nx.dot(H));
				const float YdotH = std::abs(point.Ny.dot(H));

				D = Microfacet::ndf_ggx(NdotH, XdotH, YdotH, m1, m2);
			}
			break;
		}

		pdf = 0.25f * D / (NdotV * NdotL);
		return (0.25f * D * G) / (NdotV * NdotL);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		float F = fresnelTerm(in.Point);

		SpectralBlob Diff = SpectralBlob::Zero();
		if (mFresnelMode != FM_Conductor && F < 1) {
			Diff = PR_1_PI * mAlbedo->eval(in.Point);
		}

		SpectralBlob Spec = SpectralBlob::Zero();
		if (F > PR_EPSILON && -in.NdotL * in.Point.NdotV > PR_EPSILON) {
			Spec = mSpecularity->eval(in.Point) * evalSpec(in.Point, in.Outgoing, in.NdotL, out.PDF_S);
		}

		out.PDF_S  = PR_1_PI * (1 - F) + out.PDF_S * F;
		out.Weight = Diff * (1 - F) + Spec * F;
		out.Weight *= std::abs(in.NdotL);
		out.Type = MST_DiffuseReflection;
	}

	void sampleDiffusePath(const MaterialSampleInput& in, float u, float v, MaterialSampleOutput& out) const
	{
		out.Outgoing = Projection::cos_hemi(u, v, out.PDF_S);
		out.Outgoing = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);
		out.Weight   = PR_1_PI * mAlbedo->eval(in.Point) * std::abs(out.Outgoing.dot(in.Point.N));
		out.Type	 = MST_DiffuseReflection;
	}

	void sampleSpecularPath(const MaterialSampleInput& in, float u, float v, MaterialSampleOutput& out) const
	{
		float m1 = mRoughnessX->eval(in.Point);
		m1 *= m1;

		Vector3f O;
		switch (mDistributionMode) {
		case DM_Blinn:
			O = Microfacet::sample_ndf_blinn(u, v, m1, out.PDF_S);
			break;
		default:
		case DM_Beckmann:
			O = Microfacet::sample_ndf_beckmann(u, v, m1, out.PDF_S);
			break;
		case DM_GGX:
			if (mRoughnessX == mRoughnessY) {
				O = Microfacet::sample_ndf_ggx(u, v, m1, out.PDF_S);
			} else {
				float m2 = mRoughnessY->eval(in.Point);
				m2 *= m2;
				O = Microfacet::sample_ndf_ggx(u, v, m1, m2, out.PDF_S);
			}
			break;
		}

		Vector3f H   = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, O);
		out.Outgoing = Reflection::reflect(H.dot(in.Point.Ray.Direction), H, in.Point.Ray.Direction);
		float NdotL  = std::abs(in.Point.N.dot(out.Outgoing));
		if (NdotL > PR_EPSILON)
			out.PDF_S = std::min(std::max(out.PDF_S / (-4 * NdotL * in.Point.NdotV), 0.0f), 1.0f);
		else
			out.PDF_S = 0;

		out.Type = MST_SpecularReflection;

		float _ignore;
		out.Weight = mSpecularity->eval(in.Point) * evalSpec(in.Point, out.Outgoing, NdotL, _ignore) * NdotL;
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		float F = fresnelTerm(in.Point);

		if (in.RND[0] <= F) {
			sampleSpecularPath(in, in.RND[0] / F, in.RND[1], out);
			out.PDF_S *= F;
			out.Type = MST_SpecularReflection;
		} else if (mFresnelMode == FM_Conductor) {
			// Absorb
			out.Weight = 0.0f;
			out.PDF_S  = 0.0f;
		} else {
			sampleDiffusePath(in, (in.RND[0] - F) / (1 - F), in.RND[1], out);
			out.PDF_S *= 1.0f - F;
			out.Type = MST_DiffuseReflection;
		}

		// No translucent
		if (out.Outgoing.dot(in.Point.N) < PR_EPSILON)
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
	std::shared_ptr<FloatSpectralShadingSocket> mAlbedo;
	std::shared_ptr<FloatSpectralShadingSocket> mSpecularity;
	std::shared_ptr<FloatSpectralShadingSocket> mIOR;
	std::shared_ptr<FloatScalarShadingSocket> mRoughnessX;
	std::shared_ptr<FloatScalarShadingSocket> mRoughnessY;
	std::shared_ptr<FloatScalarShadingSocket> mConductorAbsorption;
}; // namespace PR

class MicrofacetMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;

		const std::string fMStr = params.getString("fresnel_mode", "");
		const std::string gMStr = params.getString("geometric_mode", "");
		const std::string dMStr = params.getString("distribution_mode", "");

		std::shared_ptr<FloatScalarShadingSocket> roughnessX;
		std::shared_ptr<FloatScalarShadingSocket> roughnessY;
		const auto roughnessP = params.getParameter("roughness");

		if (roughnessP.isValid()) {
			roughnessX = ctx.Env->lookupScalarShadingSocket(roughnessP, 0.5f);
			roughnessY = roughnessX;
		} else {
			roughnessX = ctx.Env->lookupScalarShadingSocket(params.getParameter("roughness_x"), 0.5f);
			roughnessY = ctx.Env->lookupScalarShadingSocket(params.getParameter("roughness_y"), 0.5f);
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
													ctx.Env->lookupSpectralShadingSocket(params.getParameter("albedo"), 1),
													ctx.Env->lookupSpectralShadingSocket(params.getParameter("specularity"), 1),
													ctx.Env->lookupSpectralShadingSocket(params.getParameter("index"), 1.55f),
													roughnessX, roughnessY,
													ctx.Env->lookupScalarShadingSocket(params.getParameter("conductor_absorption"), 0.5f));
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