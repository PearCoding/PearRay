#include "Environment.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/MaterialManager.h"
#include "renderer/RenderTileSession.h"

#include "Test.h"

#include <unordered_set>

using namespace PR;

inline IntersectionPoint constructTestIP(bool backside = false)
{
	Ray ray;
	ray.Origin		 = Vector3f::Zero();
	ray.Direction	 = Vector3f(1, 0, 1).normalized();
	ray.WavelengthNM = SpectralBlob(560.0f, 540.0f, 400.0f, 600.0f);

	if (!backside)
		ray.Direction *= -1;

	GeometryPoint gp;
	gp.N = Vector3f(0, 0, 1);
	Tangent::frame(gp.N, gp.Nx, gp.Ny);
	gp.UV		   = Vector2f(0.5, 0.5);
	gp.dUV		   = Vector2f::Zero();
	gp.PrimitiveID = 0;
	gp.EmissionID  = PR_INVALID_ID;
	gp.DisplaceID  = PR_INVALID_ID;
	gp.EntityID	   = 0;
	gp.MaterialID  = 0;

	return IntersectionPoint::forSurface(ray, Vector3f(0, 0, 0.5f), gp);
}

inline ParameterGroup prepareParameters(const std::string& name)
{
	ParameterGroup params;

	if (name.find("rough") != std::string::npos) {
		params.addParameter("roughness", Parameter::fromNumber(0.164f));
	}

	return params;
}

inline void checkEvalPdf(PRT::Test* _test, const IntersectionPoint& ip, const Vector3f& L)
{
	// Maybe will only work for embedded plugins?
	const auto env	 = Environment::createQueryEnvironment("./");
	const auto manag = env->materialManager();

	std::unordered_set<std::shared_ptr<IMaterialPlugin>> plugins;
	for (const auto& fac : manag->factoryMap())
		plugins.insert(fac.second);

	SceneLoadContext ctx(env.get());

	RenderTileSession session;
	for (const auto& fac : plugins) {
		const std::string name = fac->getNames().front();
		ctx.parameters()	   = prepareParameters(name);
		const auto material	   = fac->create(name, ctx);

		PR_MESSAGE("Material: " + name);

		if (!material || material->hasOnlyDeltaDistribution())
			continue;

		MaterialEvalInput min;
		min.Context		   = MaterialEvalContext::fromIP(ip, L);
		min.ShadingContext = ShadingContext::fromIP(0, ip);

		MaterialEvalOutput mout;
		material->eval(min, mout, session);

		MaterialPDFOutput pout;
		material->pdf(min, pout, session);

		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			PR_CHECK_NEARLY_EQ(mout.PDF_S[i], pout.PDF_S[i]);
	}
}

inline void checkSampleEval(PRT::Test* _test, const IntersectionPoint& ip, const Vector2f& rnd)
{
	// Maybe will only work for embedded plugins?
	const auto env	 = Environment::createQueryEnvironment("./");
	const auto manag = env->materialManager();

	std::unordered_set<std::shared_ptr<IMaterialPlugin>> plugins;
	for (const auto& fac : manag->factoryMap())
		plugins.insert(fac.second);

	SceneLoadContext ctx(env.get());

	RenderTileSession session;
	for (const auto& fac : plugins) {
		const std::string name = fac->getNames().front();
		ctx.parameters()	   = prepareParameters(name);
		const auto material	   = fac->create(name, ctx);

		PR_MESSAGE("Material: " + name);
		if (!material)
			continue;

		MaterialSampleInput min;
		min.Context		   = MaterialSampleContext::fromIP(ip);
		min.ShadingContext = ShadingContext::fromIP(0, ip);
		min.RND			   = rnd;

		MaterialSampleOutput mout;
		material->sample(min, mout, session);

		// Check if really the delta flag is set
		if (material->hasOnlyDeltaDistribution()) {
			PR_CHECK_TRUE(mout.isDelta());
			continue;
		}

		// No evaluation if delta!
		if (mout.isDelta())
			continue;

		MaterialEvalInput ein;
		ein.Context		   = MaterialEvalContext::fromIP(ip, mout.globalL(ip));
		ein.ShadingContext = min.ShadingContext;

		MaterialEvalOutput eout;
		material->eval(ein, eout, session);

		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			PR_CHECK_NEARLY_EQ(mout.PDF_S[i], eout.PDF_S[i]);

		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			PR_CHECK_NEARLY_EQ(mout.Weight[i], eout.Weight[i]);
	}
}

PR_BEGIN_TESTCASE(Materials)
PR_TEST("[Front] Eval = PDF")
{
	const IntersectionPoint ip = constructTestIP(false);
	const Vector3f L		   = Vector3f(0, 1, 1).normalized();
	checkEvalPdf(_test, ip, L);
}

PR_TEST("[Front] Eval = Sample")
{
	const IntersectionPoint ip = constructTestIP(false);
	const Vector2f rnd		   = Vector2f(0.15789f, 0.424242f);
	checkSampleEval(_test, ip, rnd);
}

PR_TEST("[Back] Eval = PDF")
{
	const IntersectionPoint ip = constructTestIP(true);
	const Vector3f L		   = Vector3f(0, 1, 1).normalized();
	checkEvalPdf(_test, ip, L);
}

PR_TEST("[Back] Eval = Sample")
{
	const IntersectionPoint ip = constructTestIP(true);
	const Vector2f rnd		   = Vector2f(0.15789f, 0.424242f);
	checkSampleEval(_test, ip, rnd);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Materials);
PRT_END_MAIN