#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Microfacet.h"
#include "math/Projection.h"
#include "math/Reflection.h"
#include "math/Spherical.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

class WardMaterial : public IMaterial {
public:
	WardMaterial(uint32 id,
				 const std::shared_ptr<FloatSpectralNode>& alb,
				 const std::shared_ptr<FloatSpectralNode>& spec,
				 const std::shared_ptr<FloatScalarNode>& reflectivity,
				 const std::shared_ptr<FloatScalarNode>& roughnessX,
				 const std::shared_ptr<FloatScalarNode>& roughnessY)
		: IMaterial(id)
		, mAlbedo(alb)
		, mSpecularity(spec)
		, mReflectivity(reflectivity)
		, mRoughnessX(roughnessX)
		, mRoughnessY(roughnessY)
	{
	}

	virtual ~WardMaterial() = default;

	void startGroup(size_t, const RenderTileSession&) override
	{
	}

	void endGroup() override
	{
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto& sctx = in.ShadingContext;

		float spec = std::min(1.0f,
							  Microfacet::ward(mRoughnessX->eval(sctx),
											   mRoughnessY->eval(sctx),
											   in.Context.NdotV(), in.Context.NdotL(), in.Context.NdotH(),
											   in.Context.XdotH(), in.Context.YdotH()));

		const float refl = mReflectivity->eval(sctx);
		out.Weight		 = mAlbedo->eval(sctx) * (1 - refl) + spec * mSpecularity->eval(sctx) * refl;
		out.Weight *= std::abs(in.Context.NdotL());

		out.PDF_S = std::min(std::max(Projection::cos_hemi_pdf(in.Context.NdotL()) * (1 - refl) + spec * refl, 0.0f), 1.0f);

		out.Type = MST_DiffuseReflection;
	}

	void sampleDiffusePath(const MaterialSampleInput& in, const ShadingContext& sctx, MaterialSampleOutput& out) const
	{
		float pdf_s;
		out.L	   = Projection::cos_hemi(in.RND[0], in.RND[1], pdf_s);
		out.PDF_S  = pdf_s;
		out.Weight = mAlbedo->eval(sctx) * std::abs(out.L[2]);
		out.Type   = MST_DiffuseReflection;
	}

	void sampleSpecularPath(const MaterialSampleInput& in, const ShadingContext& sctx, MaterialSampleOutput& out) const
	{
		const float m1 = mRoughnessX->eval(sctx);
		const float m2 = mRoughnessY->eval(sctx);

		float cosTheta, sinTheta;			 // V samples
		float cosPhi, sinPhi;				 // U samples
		if (std::abs(m1 - m2) <= PR_EPSILON) // Isotropic
		{
			sinPhi = std::sin(2 * PR_PI * in.RND[0]);
			cosPhi = std::cos(2 * PR_PI * in.RND[0]);

			const float f = -std::log(std::max(0.00001f, in.RND[1])) * m1 * m1;
			cosTheta	  = 1 / (1 + f);
			sinTheta	  = std::sqrt(f) * cosTheta;

			const float t = 4 * PR_PI * m1 * m1 * cosTheta * cosTheta * cosTheta * in.RND[1];
			if (t <= PR_EPSILON)
				out.PDF_S = 1;
			else
				out.PDF_S = 1 / t;
		} else {
			const float pm1 = m1 * m1;
			const float pm2 = m2 * m2;

			const float f1 = (m2 / m1) * std::tan(2 * PR_PI * in.RND[0]);
			cosPhi		   = 1 / std::sqrt(1 + f1 * f1);
			sinPhi		   = f1 * cosPhi;

			const float cosPhi2 = cosPhi * cosPhi;
			const float tz		= (cosPhi2 / pm1 + sinPhi * sinPhi / pm2);
			const float f2		= -std::log(std::max(0.000001f, in.RND[1])) / tz;
			cosTheta			= 1 / (1 + f2);
			sinTheta			= std::sqrt(f2) * cosTheta;

			const float cosTheta2 = cosTheta * cosTheta;
			const float tu		  = pm1 * sinPhi * sinPhi + pm2 * cosPhi2;
			const float tb		  = 4 * PR_PI * m1 * m2 * (pm1 * (1 - cosPhi2) / cosPhi + pm2 * cosPhi) * cosTheta2;
			out.PDF_S			  = tu / tb * std::exp(-tz * (1 - cosTheta2) / (cosTheta2));
		}

		Vector3f H = Spherical::cartesian(sinTheta, cosTheta, sinPhi, cosPhi);
		out.L	   = Reflection::reflect(in.Context.V, H);
		out.Type   = MST_SpecularReflection;
		out.Weight = mSpecularity->eval(sctx) * out.PDF_S * std::abs(out.L[2]);
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto& sctx = in.ShadingContext;
		const float refl = mReflectivity->eval(sctx);

		if (in.RND[0] <= refl) {
			sampleSpecularPath(in, sctx, out);
			out.PDF_S /= refl;
		} else {
			sampleDiffusePath(in, sctx, out);
			out.PDF_S /= 1.0f - refl;
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <WardMaterial>:" << std::endl
			   << "    Albedo:       " << (mAlbedo ? mAlbedo->dumpInformation() : "NONE") << std::endl
			   << "    Specularity:  " << (mSpecularity ? mSpecularity->dumpInformation() : "NONE") << std::endl
			   << "    Reflectivity: " << (mReflectivity ? mReflectivity->dumpInformation() : "NONE") << std::endl
			   << "    RoughnessX:   " << (mRoughnessX ? mRoughnessX->dumpInformation() : "NONE") << std::endl
			   << "    RoughnessY:   " << (mRoughnessY ? mRoughnessY->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mAlbedo;
	std::shared_ptr<FloatSpectralNode> mSpecularity;
	std::shared_ptr<FloatScalarNode> mReflectivity;
	std::shared_ptr<FloatScalarNode> mRoughnessX;
	std::shared_ptr<FloatScalarNode> mRoughnessY;
};

class WardMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.Parameters;

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

		return std::make_shared<WardMaterial>(id,
											  ctx.Env->lookupSpectralNode(params.getParameter("albedo"), 1),
											  ctx.Env->lookupSpectralNode(params.getParameter("specularity"), 1),
											  ctx.Env->lookupScalarNode(params.getParameter("reflectivity"), 1),
											  roughnessX, roughnessY);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "ward" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::WardMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)