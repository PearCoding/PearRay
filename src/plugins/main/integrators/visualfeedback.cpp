#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "integrator/IIntegratorPlugin.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Tangent.h"

#include "path/LightPath.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "shader/ShadingPoint.h"

#include "Logger.h"

#include <boost/algorithm/string/predicate.hpp>

namespace PR {
enum VisualFeedbackMode {
	VFM_ColoredEntityID,
	VFM_ColoredMaterialID,
	VFM_ColoredEmissionID,
	VFM_ColoredDisplaceID,
	VFM_ColoredPrimitiveID,
	VFM_ColoredRayID,
	VFM_ColoredContainerID,
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
	{ "colored_container_id", VFM_ColoredContainerID },
	{ "ray_direction", VFM_RayDirection },
	{ "parameter", VFM_Parameter },
	{ "inside", VFM_Inside },
	{ "ndotv", VFM_NdotV },
	{ "validate_material", VFM_ValidateMaterial },
	{ "", VFM_ColoredEntityID }
};

constexpr int RANDOM_COLOR_COUNT						= 23;
static const Vector3f sRandomColors[RANDOM_COLOR_COUNT] = {
	Vector3f(0.450000f, 0.376630f, 0.112500f),
	Vector3f(0.112500f, 0.450000f, 0.405978f),
	Vector3f(0.112500f, 0.450000f, 0.229891f),
	Vector3f(0.450000f, 0.112500f, 0.376630f),
	Vector3f(0.435326f, 0.450000f, 0.112500f),
	Vector3f(0.112500f, 0.141848f, 0.450000f),
	Vector3f(0.435326f, 0.112500f, 0.450000f),
	Vector3f(0.112500f, 0.450000f, 0.141848f),
	Vector3f(0.347283f, 0.450000f, 0.112500f),
	Vector3f(0.450000f, 0.112500f, 0.200543f),
	Vector3f(0.112500f, 0.229891f, 0.450000f),
	Vector3f(0.450000f, 0.288587f, 0.112500f),
	Vector3f(0.347283f, 0.112500f, 0.450000f),
	Vector3f(0.450000f, 0.112500f, 0.288587f),
	Vector3f(0.450000f, 0.112500f, 0.112500f),
	Vector3f(0.450000f, 0.200543f, 0.112500f),
	Vector3f(0.171196f, 0.450000f, 0.112500f),
	Vector3f(0.112500f, 0.450000f, 0.317935f),
	Vector3f(0.259239f, 0.450000f, 0.112500f),
	Vector3f(0.259239f, 0.112500f, 0.450000f),
	Vector3f(0.112500f, 0.405978f, 0.450000f),
	Vector3f(0.171196f, 0.112500f, 0.450000f),
	Vector3f(0.112500f, 0.317935f, 0.450000f)
};

static const Vector3f TrueColor	 = Vector3f(0, 1, 0);
static const Vector3f FalseColor = Vector3f(1, 0, 0);

class IntVF : public IIntegrator {
public:
	explicit IntVF(VisualFeedbackMode mode, bool applyDot)
		: IIntegrator()
		, mMode(mode)
		, mApplyDot(applyDot)
	{
	}

	virtual ~IntVF() = default;

	// Per thread
	void onPass(RenderTileSession& session, uint32) override
	{
		PR_PROFILE_THIS;

		LightPath stdPath = LightPath::createCDL(1);
		Random random(42);

		while (session.handleCameraRays()) {
			session.handleHits(
				[&](size_t, const Ray&) {
					session.tile()->statistics().addBackgroundHitCount();
				},
				[&](const HitEntry& hitEntry,
					const Ray& ray, const GeometryPoint& pt,
					IEntity* entity, IMaterial* material) {
					PR_PROFILE_THIS;
					session.tile()->statistics().addEntityHitCount();

					ShadingPoint spt;
					spt.setByIdentity(ray, pt);
					spt.EntityID = entity->id();

					ColorTriplet radiance = ColorTriplet::Zero();
					ColorTriplet weight	  = ColorTriplet::Ones() * abs(spt.NdotV);
					switch (mMode) {
					case VFM_ColoredEntityID:
						radiance = sRandomColors[hitEntry.EntityID % RANDOM_COLOR_COUNT];
						if (mApplyDot)
							radiance *= weight;
						break;
					case VFM_ColoredMaterialID:
						radiance = sRandomColors[hitEntry.MaterialID % RANDOM_COLOR_COUNT];
						if (mApplyDot)
							radiance *= weight;
						break;
					case VFM_ColoredEmissionID:
						radiance = sRandomColors[pt.EmissionID % RANDOM_COLOR_COUNT];
						if (mApplyDot)
							radiance *= weight;
						break;
					case VFM_ColoredDisplaceID:
						radiance = sRandomColors[pt.DisplaceID % RANDOM_COLOR_COUNT];
						if (mApplyDot)
							radiance *= weight;
						break;
					case VFM_ColoredPrimitiveID:
						radiance = sRandomColors[hitEntry.PrimitiveID % RANDOM_COLOR_COUNT];
						if (mApplyDot)
							radiance *= weight;
						break;
					case VFM_ColoredRayID:
						radiance = sRandomColors[hitEntry.RayID % RANDOM_COLOR_COUNT];
						if (mApplyDot)
							radiance *= weight;
						break;
					case VFM_ColoredContainerID:
						radiance = sRandomColors[entity->containerID() % RANDOM_COLOR_COUNT];
						if (mApplyDot)
							radiance *= weight;
						break;
					case VFM_RayDirection: {
						Vector3f rescaledDir = 0.5f * (spt.Ray.Direction + Vector3f(1, 1, 1));
						radiance			 = ColorTriplet{ rescaledDir[0], rescaledDir[1], rescaledDir[2] };
						if (mApplyDot)
							radiance *= weight;
					} break;
					case VFM_Parameter:
						radiance = hitEntry.Parameter;
						if (mApplyDot)
							radiance *= weight;
						break;
					case VFM_Inside:
						radiance = (spt.Flags & SPF_Inside) ? TrueColor : FalseColor;
						if (mApplyDot)
							radiance *= weight;
						break;
					case VFM_NdotV: {
						const float originalNdotV = spt.Ray.Direction.dot(spt.Geometry.N);
						if (originalNdotV < 0)
							radiance = ColorTriplet{ 0, -originalNdotV, 0 };
						else
							radiance = ColorTriplet{ originalNdotV, 0, 0 };
					} break;
					case VFM_ValidateMaterial: {
						if (material) {
							MaterialSampleInput samp_in;
							samp_in.Point = spt;
							samp_in.RND	  = random.get2D();
							MaterialSampleOutput samp_out;
							material->sample(samp_in, samp_out, session);

							MaterialEvalInput eval_in;
							eval_in.Point	 = spt;
							eval_in.Outgoing = samp_out.Outgoing;
							eval_in.NdotL	 = spt.N.dot(eval_in.Outgoing);
							MaterialEvalOutput eval_out;
							material->eval(eval_in, eval_out, session);

							const float weightDiff = (samp_out.Weight - eval_out.Weight).cwiseAbs().sum();
							const float relPDFDiff = std::abs(samp_out.PDF_S - eval_out.PDF_S);

							radiance = ColorTriplet{ 1 - weightDiff, 1 - relPDFDiff, 1 };
						}
					}
					}

					session.pushSPFragment(spt, stdPath);
					session.pushSpectralFragment(radiance, ray, stdPath);
				});
		}
	}

	RenderStatus status() const override
	{
		return RenderStatus();
	}

private:
	VisualFeedbackMode mMode;
	bool mApplyDot;
};

class IntVFFactory : public IIntegratorFactory {
public:
	explicit IntVFFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		std::string modeS		= mParams.getString("mode", "");
		VisualFeedbackMode mode = VFM_Parameter;
		for (int i = 0; _mode[i].Name; ++i) {
			if (boost::iequals(_mode[i].Name, modeS)) {
				mode = _mode[i].Mode;
				break;
			}
		}
		return std::make_shared<IntVF>(mode, mParams.getBool("weighting", true));
	}

private:
	ParameterGroup mParams;
};

class IntVFFactoryFactory : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntVFFactory>(ctx.Parameters);
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