#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "buffer/Feedback.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "integrator/IIntegratorPlugin.h"
#include "material/IMaterial.h"
#include "math/ImportanceSampling.h"

#include "path/LightPath.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "shader/ShadingPoint.h"

#include "Logger.h"

#define MSI(n1, p1, n2, p2) IS::balance_term((n1), (p1), (n2), (p2))

/* Based on Path Integral formulation: (here length k)
 * f(x) is the brdf. In our case a material already includes the cosine factor f(x)cos(N,x)
 *
 * L(x0->x1)G(x0<->x1)f(x1)/p(x0)
 * |*| f(x2)cos(N2,x1<-x2)/p_(x1|x2)
 * |*| ...
 * |*| f(x(k-1))cos(N(k-1),x(k-2)<-x(k-1))/p_(x(k-2)|x(k-1))
 * |*| W(xk)
 *
 * p(xi) is the pdf with respect to the area
 * p_(xi) is the pdf with respect to the solid angle (not projected solid angle!)
 */
namespace PR {
class IntDirect : public IIntegrator {
public:
	explicit IntDirect(size_t lightSamples, size_t maxRayDepth)
		: IIntegrator()
		, mLightSampleCount(lightSamples)
		, mMaxRayDepth(maxRayDepth)
		, mMSIEnabled(true)
	{
	}

	virtual ~IntDirect() = default;

	ColorTriplet infiniteLight(RenderTileSession& session, const ShadingPoint& spt,
							   LightPathToken& token,
							   IInfiniteLight* infLight, IMaterial* material)
	{
		PR_PROFILE_THIS;

		ColorTriplet LiL = ColorTriplet::Zero();
		float msiL		 = 0.0f;
		ColorTriplet LiS = ColorTriplet::Zero();
		float msiS		 = 0.0f;

		const bool allowMSI = mMSIEnabled && !infLight->hasDeltaDistribution();

		// (1) Sample light
		{
			InfiniteLightSampleInput inL;
			inL.Point = spt;
			inL.RND   = session.tile()->random().get2D();
			InfiniteLightSampleOutput outL;
			infLight->sample(inL, outL, session);

			if (infLight->hasDeltaDistribution())
				outL.PDF_S = 1;

			if (outL.PDF_S > PR_EPSILON) {
				// Trace shadow ray
				Ray shadow			= spt.Ray.next(spt.P, outL.Outgoing);
				shadow.Flags		= RF_Shadow | (spt.Ray.Flags & RF_Monochrome);
				ShadowHit shadowHit = session.traceShadowRay(shadow);
				if (!shadowHit.Successful) {
					// Evaluate surface
					MaterialEvalInput in;
					in.Point	= spt;
					in.Outgoing = outL.Outgoing;
					in.NdotL	= outL.Outgoing.dot(spt.N);
					MaterialEvalOutput out;
					material->eval(in, out, session);
					token = LightPathToken(out.Type);

					msiL = allowMSI ? MSI(1, outL.PDF_S, 1, out.PDF_S) : 1.0f;
					LiL  = outL.Weight * out.Weight / outL.PDF_S;
				}
			}
		}

		if (!allowMSI)
			return LiL;

		// (2) Sample BRDF
		{
			MaterialSampleInput inS;
			inS.Point = spt;
			inS.RND   = session.tile()->random().get2D();
			MaterialSampleOutput outS;
			material->sample(inS, outS, session);
			token = LightPathToken(outS.Type);

			if (outS.PDF_S > PR_EPSILON
				&& !outS.Weight.isZero()) {

				// Trace shadow ray
				Ray shadow			= spt.Ray.next(spt.P, outS.Outgoing);
				shadow.Flags		= RF_Shadow | (spt.Ray.Flags & RF_Monochrome);
				ShadowHit shadowHit = session.traceShadowRay(shadow);
				if (!shadowHit.Successful) {
					InfiniteLightEvalInput inL;
					inL.Point = spt;
					InfiniteLightEvalOutput outL;
					infLight->eval(inL, outL, session);

					msiS = MSI(1, outL.PDF_S, 1, outS.PDF_S);
					LiS  = outL.Weight * outS.Weight / outS.PDF_S;
				}
			}
		}

		if (msiS <= PR_EPSILON || LiS.isZero())
			return LiL;
		else if (msiL <= PR_EPSILON || LiL.isZero())
			return LiS;
		else
			return LiL * msiL + LiS * msiS;
	}

	void scatterLight(RenderTileSession& session, LightPath& path,
					  const ShadingPoint& spt,
					  IMaterial* material,
					  bool& infLightHandled)
	{
		PR_PROFILE_THIS;

		infLightHandled = false;

		constexpr int MIN_DEPTH			= 4; //Minimum level of depth after which russian roulette will apply
		constexpr float DEPTH_DROP_RATE = 0.90f;

		const float scatProb = std::min<float>(1.0f, pow(DEPTH_DROP_RATE, (int)spt.Ray.IterationDepth - MIN_DEPTH));

		// Sample BxDF
		MaterialSampleInput in;
		in.Point = spt;
		in.RND   = session.tile()->random().get2D();
		MaterialSampleOutput out;
		material->sample(in, out, session);

		if (material->hasDeltaDistribution())
			out.PDF_S = 1;

		//const float NdotL = out.Outgoing.dot(spt.N);
		if (out.PDF_S > PR_EPSILON
			&& spt.Ray.IterationDepth + 1 < mMaxRayDepth
			&& session.tile()->random().getFloat() <= scatProb) {
			Ray next = spt.Ray.next(spt.P, out.Outgoing);
			next.Weight *= out.Weight / (scatProb * out.PDF_S);
			next.Flags = RF_Bounce | (spt.Ray.Flags & RF_Monochrome);

			if (!next.Weight.isZero()) {
				path.addToken(LightPathToken(out.Type)); // (1)

				GeometryPoint npt;
				IEntity* nentity;
				IMaterial* nmaterial;
				if (session.traceBounceRay(next, npt, nentity, nmaterial)) {
					eval(session, path, next, npt, nentity, nmaterial);
				} else {
					session.tile()->statistics().addBackgroundHitCount();
					infLightHandled = true;

					path.addToken(LightPathToken::Background());
					for (auto light : session.tile()->context()->scene()->infiniteLights()) {
						if (light->hasDeltaDistribution())
							continue;

						// Only the ray is fixed (Should be handled better!)
						InfiniteLightEvalInput lin;
						lin.Point.Ray   = next;
						lin.Point.Flags = SPF_Background;
						InfiniteLightEvalOutput lout;
						light->eval(lin, lout, session);

						session.pushSpectralFragment(lout.Weight, next, path);
					}
					path.popToken();
				}

				path.popToken(); // (1)
			}
		}
	}

	ColorTriplet directLight(RenderTileSession& session, const ShadingPoint& spt,
							 LightPathToken& token,
							 IMaterial* material)
	{
		PR_PROFILE_THIS;

		ColorTriplet LiL = ColorTriplet::Zero();
		float msiL		 = 0.0f;
		ColorTriplet LiS = ColorTriplet::Zero();
		float msiS		 = 0.0f;

		// Pick light and point
		float pdfA;
		GeometryPoint lightPt;
		IEntity* light = session.pickRandomLight(spt.N, lightPt, pdfA);
		if (!light)
			return LiL;
		IEmission* ems = session.getEmission(lightPt.EmissionID);
		if (!ems) {
			session.pushFeedbackFragment(OF_MissingEmission, spt.Ray);
			return LiL;
		}

		// (1) Sample light
		{
			Vector3f L		 = (lightPt.P - spt.P);
			const float sqrD = L.squaredNorm();
			L.normalize();
			const float cosO = std::abs(L.dot(lightPt.N));
			const float cosI = L.dot(spt.N);

			if (std::abs(cosI) >= PR_EPSILON
				&& pdfA >= PR_EPSILON) {

				// Trace shadow ray
				Ray shadow			= spt.Ray.next(spt.P, L);
				shadow.Flags		= RF_Shadow | (spt.Ray.Flags & RF_Monochrome);
				ShadowHit shadowHit = session.traceShadowRay(shadow);

				if (shadowHit.Successful && shadowHit.EntityID == light->id()) {
					// Evaluate light
					LightEvalInput inL;
					inL.Entity = light;
					inL.Point.setByIdentity(shadow, lightPt);
					LightEvalOutput outL;
					ems->eval(inL, outL, session);

					// Evaluate surface
					MaterialEvalInput in;
					in.Point	= spt;
					in.Outgoing = L;
					in.NdotL	= cosI;
					MaterialEvalOutput out;
					material->eval(in, out, session);

					const float pdfS = IS::toSolidAngle(pdfA, sqrD, cosO);
					msiL			 = mMSIEnabled ? MSI(1, pdfS, 1, out.PDF_S) : 1.0f;
					LiL				 = outL.Weight * out.Weight / pdfA;
				}
			}
		}

		if (!mMSIEnabled)
			return LiL;

		// (2) Sample BRDF
		{
			MaterialSampleInput inS;
			inS.Point = spt;
			inS.RND   = session.tile()->random().get2D();
			MaterialSampleOutput outS;
			material->sample(inS, outS, session);
			token = LightPathToken(outS.Type);

			if (outS.PDF_S > PR_EPSILON
				&& !outS.Weight.isZero()) {

				// Trace shadow ray
				Ray shadow			= spt.Ray.next(spt.P, outS.Outgoing);
				shadow.Flags		= RF_Shadow | (spt.Ray.Flags & RF_Monochrome);
				ShadowHit shadowHit = session.traceShadowRay(shadow);
				if (shadowHit.Successful
					&& shadowHit.EntityID == light->id()) {
					// Retrive geometry information from the new point
					GeometryPoint nlightPt;
					light->provideGeometryPoint(
						shadow.Direction, shadowHit.PrimitiveID,
						Vector3f(shadowHit.Parameter[0], shadowHit.Parameter[1], shadowHit.Parameter[2]), nlightPt);

					if (std::abs(shadow.Direction.dot(nlightPt.N)) > PR_EPSILON) {
						// Evaluate light
						LightEvalInput inL;
						inL.Entity = light;
						inL.Point.setByIdentity(shadow, nlightPt);
						LightEvalOutput outL;
						ems->eval(inL, outL, session);

						const float pdfS = IS::toSolidAngle(
							1 / light->surfaceArea(),
							(shadow.Origin - nlightPt.P).squaredNorm(),
							std::abs(shadow.Direction.dot(nlightPt.N)));
						msiS = MSI(1, pdfS, 1, outS.PDF_S);
						LiS  = outL.Weight * outS.Weight / outS.PDF_S;
					}
				}
			}
		}

		if (msiS <= PR_EPSILON || LiS.isZero())
			return LiL;
		else if (msiL <= PR_EPSILON || LiL.isZero())
			return LiS;
		else
			return LiL * msiL + LiS * msiS;
	}

	void eval(RenderTileSession& session, LightPath& path,
			  const Ray& ray, const GeometryPoint& pt,
			  IEntity* entity, IMaterial* material)
	{
		PR_PROFILE_THIS;

		session.tile()->statistics().addEntityHitCount();

		// Early drop out for invalid splashes
		if (!entity->isLight() && !material)
			return;

		ShadingPoint spt;
		spt.setByIdentity(ray, pt);
		spt.EntityID = entity->id();
		session.pushSPFragment(spt, path);

		// Only consider camera rays, as everything else produces too much noise
		if (entity->isLight()
			&& spt.Ray.IterationDepth == 0
			&& spt.Depth2 > PR_EPSILON) {
			IEmission* ems = session.getEmission(spt.Geometry.EmissionID);
			if (ems) {
				// Evaluate light
				LightEvalInput inL;
				inL.Entity = entity;
				inL.Point  = spt;
				LightEvalOutput outL;
				ems->eval(inL, outL, session);

				const float prevGEOM  = /*std::abs(spt.NdotV) * std::abs(spt.Ray.NdotL)*/ 1 /* spt.Depth2*/;
				ColorTriplet radiance = outL.Weight * prevGEOM;

				if (!radiance.isZero()) {
					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(radiance, spt.Ray, path);
					path.popToken();
				}
			}
		}

		if (!material)
			return;

		// Scatter Light
		bool infLightHandled;
		scatterLight(session, path, spt, material, infLightHandled);

		if (!material->hasDeltaDistribution()) {
			// Direct Light
			const float factor = 1.0f / mLightSampleCount;
			for (size_t i = 0; i < mLightSampleCount; ++i) {
				LightPathToken token;
				ColorTriplet radiance = factor * directLight(session, spt, token, material);

				if (!radiance.isZero()) {
					path.addToken(token);
					path.addToken(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushSpectralFragment(radiance, spt.Ray, path);
					path.popToken(2);
				}
			}

			// Infinite Lights
			for (auto light : session.tile()->context()->scene()->infiniteLights()) {
				if (infLightHandled && !light->hasDeltaDistribution()) // Already handled in scatter equation
					continue;

				LightPathToken token;
				ColorTriplet radiance = infiniteLight(session, spt, token,
													  light.get(), material);

				if (!radiance.isZero()) {
					path.addToken(token);
					path.addToken(LightPathToken::Background());
					session.pushSpectralFragment(radiance, spt.Ray, path);
					path.popToken(2);
				}
			}
		}
	}

	// Per thread
	void onPass(RenderTileSession& session, uint32) override
	{
		PR_PROFILE_THIS;

		const size_t maxPathSize = mMaxRayDepth + 2;

		LightPath cb = LightPath::createCB();
		LightPath path(maxPathSize);
		path.addToken(LightPathToken::Camera());
		while (session.handleCameraRays()) {
			session.handleHits(
				[&](size_t /*session_ray_id*/, const Ray& ray) {
					PR_PROFILE_THIS;
					session.tile()->statistics().addBackgroundHitCount();

					for (auto light : session.tile()->context()->scene()->infiniteLights()) {
						if (light->hasDeltaDistribution())
							continue;

						// Only the ray is fixed (Should be handled better!)
						InfiniteLightEvalInput in;
						in.Point.Ray   = ray;
						in.Point.Flags = SPF_Background;
						InfiniteLightEvalOutput out;
						light->eval(in, out, session);

						session.pushSpectralFragment(out.Weight, ray, cb);
					}
				},
				[&](const HitEntry& /*entry*/,
					const Ray& ray, const GeometryPoint& pt,
					IEntity* entity, IMaterial* material) {
					eval(session, path, ray, pt, entity, material);
					PR_ASSERT(path.currentSize() == 1, "Add/Pop count does not match!");
				});
		}
	}

	RenderStatus status() const override
	{
		return RenderStatus();
	}

	inline void enableMSI(bool b) { mMSIEnabled = b; }

private:
	const size_t mLightSampleCount;
	const size_t mMaxRayDepth;
	bool mMSIEnabled;
};

class IntDirectFactory : public IIntegratorFactory {
public:
	explicit IntDirectFactory(const ParameterGroup& params)
		: mParams(params)
	{
	}

	std::shared_ptr<IIntegrator> createInstance() const override
	{
		size_t lightsamples = (size_t)mParams.getUInt("light_sample_count", 1);
		size_t maxraydepth  = (size_t)mParams.getUInt("max_ray_depth", 4);

		auto obj = std::make_shared<IntDirect>(lightsamples, maxraydepth);
		obj->enableMSI(mParams.getBool("msi", true));
		return obj;
	}

private:
	ParameterGroup mParams;
};

class IntDirectFactoryFactory : public IIntegratorPlugin {
public:
	std::shared_ptr<IIntegratorFactory> create(uint32, const SceneLoadContext& ctx) override
	{
		return std::make_shared<IntDirectFactory>(ctx.Parameters);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "direct", "standard", "default" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};

} // namespace PR

PR_PLUGIN_INIT(PR::IntDirectFactoryFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)