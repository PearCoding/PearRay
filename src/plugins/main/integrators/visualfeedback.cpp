#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "integrator/IIntegratorPlugin.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "spectral/SpectralUpsampler.h"

#include "path/LightPath.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "renderer/StreamPipeline.h"
#include "trace/IntersectionPoint.h"

#include "Logger.h"

namespace PR {
enum VisualFeedbackMode {
	VFM_ColoredEntityID,
	VFM_ColoredMaterialID,
	VFM_ColoredEmissionID,
	VFM_ColoredDisplaceID,
	VFM_ColoredPrimitiveID,
	VFM_ColoredRayID,
	VFM_RayDirection,
	VFM_Parameter,
	VFM_Inside,
	VFM_NdotV,
	VFM_ValidateMaterial
};

static struct {
	const char* Name;
	VisualFeedbackMode Mode;
} _mode[] = {
	{ "colored_entity_id", VFM_ColoredEntityID },
	{ "colored_material_id", VFM_ColoredMaterialID },
	{ "colored_emission_id", VFM_ColoredEmissionID },
	{ "colored_displace_id", VFM_ColoredDisplaceID },
	{ "colored_primitive_id", VFM_ColoredPrimitiveID },
	{ "colored_ray_id", VFM_ColoredRayID },
	{ "ray_direction", VFM_RayDirection },
	{ "parameter", VFM_Parameter },
	{ "inside", VFM_Inside },
	{ "ndotv", VFM_NdotV },
	{ "validate_material", VFM_ValidateMaterial },
	{ "", VFM_ColoredEntityID }
};

using RGB = std::array<float, 3>;

constexpr int RANDOM_COLOR_COUNT				   = 23;
static const RGB sRandomColors[RANDOM_COLOR_COUNT] = {
	{ 0.450000f, 0.376630f, 0.112500f },
	{ 0.112500f, 0.450000f, 0.405978f },
	{ 0.112500f, 0.450000f, 0.229891f },
	{ 0.450000f, 0.112500f, 0.376630f },
	{ 0.435326f, 0.450000f, 0.112500f },
	{ 0.112500f, 0.141848f, 0.450000f },
	{ 0.435326f, 0.112500f, 0.450000f },
	{ 0.112500f, 0.450000f, 0.141848f },
	{ 0.347283f, 0.450000f, 0.112500f },
	{ 0.450000f, 0.112500f, 0.200543f },
	{ 0.112500f, 0.229891f, 0.450000f },
	{ 0.450000f, 0.288587f, 0.112500f },
	{ 0.347283f, 0.112500f, 0.450000f },
	{ 0.450000f, 0.112500f, 0.288587f },
	{ 0.450000f, 0.112500f, 0.112500f },
	{ 0.450000f, 0.200543f, 0.112500f },
	{ 0.171196f, 0.450000f, 0.112500f },
	{ 0.112500f, 0.450000f, 0.317935f },
	{ 0.259239f, 0.450000f, 0.112500f },
	{ 0.259239f, 0.112500f, 0.450000f },
	{ 0.112500f, 0.405978f, 0.450000f },
	{ 0.171196f, 0.112500f, 0.450000f },
	{ 0.112500f, 0.317935f, 0.450000f }
};

static const RGB TrueColor	= { 0, 1, 0 };
static const RGB FalseColor = { 1, 0, 0 };
static const RGB RedColor	= { 1, 0, 0 };
static const RGB GreenColor = { 0, 1, 0 };
static const RGB BlueColor	= { 0, 0, 1 };

struct VFParametricCache {
	std::array<ParametricBlob, RANDOM_COLOR_COUNT> RandomColors;
	ParametricBlob True;
	ParametricBlob False;
	ParametricBlob Red;
	ParametricBlob Green;
	ParametricBlob Blue;
};

class IntVFInstance : public IIntegratorInstance {
public:
	explicit IntVFInstance(const VFParametricCache& parametricCache, VisualFeedbackMode mode, bool applyDot)
		: mMode(mode)
		, mApplyDot(applyDot)
		, mParametric(parametricCache)
	{
	}

	virtual ~IntVFInstance() = default;

	void handleShadingGroup(RenderTileSession& session, const ShadingGroup& grp)
	{
		LightPath stdPath = LightPath::createCDL(1);
		Random random(42);
		session.tile()->statistics().addEntityHitCount(grp.size());
		for (size_t i = 0; i < grp.size(); ++i) {
			IntersectionPoint spt;
			grp.computeShadingPoint(i, spt);
			HitEntry hitEntry;
			grp.extractHitEntry(i, hitEntry);

			SpectralBlob radiance = SpectralBlob::Zero();
			SpectralBlob weight	  = SpectralBlob::Ones() * abs(spt.Surface.NdotV);
			switch (mMode) {
			case VFM_ColoredEntityID:
				radiance = SpectralUpsampler::compute(
					mParametric.RandomColors[spt.Surface.Geometry.EntityID % RANDOM_COLOR_COUNT],
					spt.Ray.WavelengthNM);
				if (mApplyDot)
					radiance *= weight;
				break;
			case VFM_ColoredMaterialID:
				radiance = SpectralUpsampler::compute(
					mParametric.RandomColors[spt.Surface.Geometry.MaterialID % RANDOM_COLOR_COUNT],
					spt.Ray.WavelengthNM);
				if (mApplyDot)
					radiance *= weight;
				break;
			case VFM_ColoredEmissionID:
				radiance = SpectralUpsampler::compute(
					mParametric.RandomColors[spt.Surface.Geometry.EmissionID % RANDOM_COLOR_COUNT],
					spt.Ray.WavelengthNM);
				if (mApplyDot)
					radiance *= weight;
				break;
			case VFM_ColoredDisplaceID:
				radiance = SpectralUpsampler::compute(
					mParametric.RandomColors[spt.Surface.Geometry.DisplaceID % RANDOM_COLOR_COUNT],
					spt.Ray.WavelengthNM);
				if (mApplyDot)
					radiance *= weight;
				break;
			case VFM_ColoredPrimitiveID:
				radiance = SpectralUpsampler::compute(
					mParametric.RandomColors[spt.Surface.Geometry.PrimitiveID % RANDOM_COLOR_COUNT],
					spt.Ray.WavelengthNM);
				if (mApplyDot)
					radiance *= weight;
				break;
			case VFM_ColoredRayID:
				radiance = SpectralUpsampler::compute(
					mParametric.RandomColors[hitEntry.RayID % RANDOM_COLOR_COUNT],
					spt.Ray.WavelengthNM);
				if (mApplyDot)
					radiance *= weight;
				break;
			case VFM_RayDirection: {
				Vector3f rescaledDir = 0.5f * (spt.Ray.Direction + Vector3f(1, 1, 1));
				const auto r		 = SpectralUpsampler::compute(mParametric.Red, spt.Ray.WavelengthNM);
				const auto g		 = SpectralUpsampler::compute(mParametric.Green, spt.Ray.WavelengthNM);
				const auto b		 = SpectralUpsampler::compute(mParametric.Blue, spt.Ray.WavelengthNM);
				radiance			 = r * rescaledDir[0] + g * rescaledDir[1] + b * rescaledDir[2];
				if (mApplyDot)
					radiance *= weight;
			} break;
			case VFM_Parameter: {
				const auto r = SpectralUpsampler::compute(mParametric.Red, spt.Ray.WavelengthNM);
				const auto g = SpectralUpsampler::compute(mParametric.Green, spt.Ray.WavelengthNM);
				const auto b = SpectralUpsampler::compute(mParametric.Blue, spt.Ray.WavelengthNM);
				radiance	 = r * hitEntry.Parameter[0] + g * hitEntry.Parameter[1] + b * hitEntry.Parameter[2];
				if (mApplyDot)
					radiance *= weight;
			} break;
			case VFM_Inside:
				radiance = SpectralUpsampler::compute(
					Scattering::is_inside_global(spt.Surface.NdotV) ? mParametric.True : mParametric.False,
					spt.Ray.WavelengthNM);
				if (mApplyDot)
					radiance *= weight;
				break;
			case VFM_NdotV: {
				const float originalNdotV = spt.Ray.Direction.dot(spt.Surface.Geometry.N);
				if (originalNdotV < 0) {
					const auto g = SpectralUpsampler::compute(mParametric.Green, spt.Ray.WavelengthNM);
					radiance	 = g * (-originalNdotV);
				} else {
					const auto r = SpectralUpsampler::compute(mParametric.Red, spt.Ray.WavelengthNM);
					radiance	 = r * originalNdotV;
				}
			} break;
			case VFM_ValidateMaterial: {
				IMaterial* mat = session.getMaterial(spt.Surface.Geometry.MaterialID);
				if (mat) {
					MaterialSampleInput samp_in{ MaterialSampleContext::fromIP(spt), ShadingContext::fromIP(session.threadID(), spt), random.get2D() };
					MaterialSampleOutput samp_out;
					mat->sample(samp_in, samp_out, session);

					MaterialEvalInput eval_in{ MaterialEvalContext::fromIP(spt, samp_out.globalL(spt)), ShadingContext::fromIP(session.threadID(), spt) };
					MaterialEvalOutput eval_out;
					mat->eval(eval_in, eval_out, session);

					MaterialEvalOutput pdf_out;
					mat->pdf(eval_in, pdf_out, session);

					float weightNorm = samp_out.Weight.cwiseAbs().sum();
					weightNorm		 = weightNorm > PR_EPSILON ? 1 / weightNorm : 1.0f;
					float pdfNorm	 = samp_out.PDF_S.cwiseAbs().sum();
					pdfNorm			 = pdfNorm > PR_EPSILON ? 1 / pdfNorm : 1.0f;

					const float weightDiff		   = (samp_out.Weight - eval_out.Weight).cwiseAbs().sum() * weightNorm;
					const float relForwardPDFDiff  = (samp_out.PDF_S - eval_out.PDF_S).cwiseAbs().sum() * pdfNorm;
					const float relBackwardPDFDiff = (samp_out.PDF_S - pdf_out.PDF_S).cwiseAbs().sum() * pdfNorm;

					const auto r = SpectralUpsampler::compute(mParametric.Red, spt.Ray.WavelengthNM);
					const auto g = SpectralUpsampler::compute(mParametric.Green, spt.Ray.WavelengthNM);
					const auto b = SpectralUpsampler::compute(mParametric.Blue, spt.Ray.WavelengthNM);
					radiance	 = r * (1 - weightDiff) + g * (1 - relForwardPDFDiff) + b * (1 - relBackwardPDFDiff);
				}
			}
			}

			session.pushSPFragment(spt, stdPath);
			session.pushSpectralFragment(SpectralBlob::Ones(), SpectralBlob::Ones(), radiance, spt.Ray, stdPath);
		}
	}

	void onTile(RenderTileSession& session) override
	{
		PR_PROFILE_THIS;

		while (!session.pipeline()->isFinished()) {
			session.pipeline()->runPipeline();
			while (session.pipeline()->hasShadingGroup()) {
				auto sg = session.pipeline()->popShadingGroup(session);
				session.tile()->statistics().addDepthCount(sg.size());
				if (sg.isBackground())
					session.tile()->statistics().addBackgroundHitCount(sg.size());
				else
					handleShadingGroup(session, sg);
			}
		}
	}

private:
	const VisualFeedbackMode mMode;
	const bool mApplyDot;
	const VFParametricCache mParametric;
};

class IntVF : public IIntegrator {
public:
	explicit IntVF(const VFParametricCache& parametricCache, VisualFeedbackMode mode, bool applyDot)
		: IIntegrator()
		, mMode(mode)
		, mApplyDot(applyDot)
		, mParametric(parametricCache)
	{
	}

	virtual ~IntVF() = default;

	std::shared_ptr<IIntegratorInstance> createThreadInstance(RenderContext*, size_t) override
	{
		return std::make_shared<IntVFInstance>(mParametric, mMode, mApplyDot);
	}

private:
	const VisualFeedbackMode mMode;
	const bool mApplyDot;
	const VFParametricCache mParametric;
};

class IntVFFactory : public IIntegratorFactory {
public:
	explicit IntVFFactory(const VFParametricCache& parametricCache, const ParameterGroup& params)
		: mParams(params)
		, mParametric(parametricCache)
	{
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		std::string modeS = mParams.getString("mode", "");
		std::transform(modeS.begin(), modeS.end(), modeS.begin(),
					   [](unsigned char c) { return std::tolower(c); });

		VisualFeedbackMode mode = VFM_Parameter;
		for (int i = 0; _mode[i].Name; ++i) {
			if (_mode[i].Name == modeS) {
				mode = _mode[i].Mode;
				break;
			}
		}
		return std::make_shared<IntVF>(mParametric, mode, mParams.getBool("weighting", true));
	}

private:
	ParameterGroup mParams;
	const VFParametricCache mParametric;
};

class IntVFFactoryFactory : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(uint32, const std::string&, const SceneLoadContext& ctx) override
	{
		// Prepare color cache
		VFParametricCache parametric;
		if (ctx.hasEnvironment()) {
			const auto mapper = ctx.environment()->defaultSpectralUpsampler();
			for (size_t i = 0; i < RANDOM_COLOR_COUNT; ++i)
				mapper->prepare(sRandomColors[i].data(), parametric.RandomColors[i].data(), 1);

			mapper->prepare(TrueColor.data(), parametric.True.data(), 1);
			mapper->prepare(FalseColor.data(), parametric.False.data(), 1);
			mapper->prepare(RedColor.data(), parametric.Red.data(), 1);
			mapper->prepare(GreenColor.data(), parametric.Green.data(), 1);
			mapper->prepare(BlueColor.data(), parametric.Blue.data(), 1);
		}

		return std::make_shared<IntVFFactory>(parametric, ctx.parameters());
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "vf", "visual", "feedback", "visual_feedback", "visualfeedback", "debug" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntVFFactoryFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)