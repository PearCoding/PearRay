#include "Environment.h"
#include "Profiler.h"
#include "buffer/Feedback.h"
#include "emission/IEmission.h"
#include "infinitelight/IInfiniteLight.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "material/IMaterial.h"
#include "math/ImportanceSampling.h"
#include "path/LightPathBuffer.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "shader/ShadingPoint.h"

#include "Logger.h"

#define MSI(n1, p1, n2, p2) IS::power_term((n1), (p1), (n2), (p2))

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
	explicit IntDirect(size_t lightSamples)
		: IIntegrator()
		, mLightSampleCount(lightSamples)
		, mMSIEnabled(true)
	{
	}

	virtual ~IntDirect() = default;

	void onInit(RenderContext* context) override
	{
		mLPBs.resize(context->threads());
	}

	void onThreadStart(RenderContext* context, size_t index) override
	{
		mLPBs[index] = std::make_unique<LightPathBuffer>(context->settings().maxParallelRays,
														 context->settings().maxRayDepth);
	}

	ColorTriplet infiniteLight(RenderTileSession& session, const ShadingPoint& spt,
							   LightPathToken& token,
							   IInfiniteLight* infLight, IMaterial* material)
	{
		PR_PROFILE_THIS;

		ColorTriplet LiL = ColorTriplet::Zero();
		float msiL = 0.0f;
		ColorTriplet LiS = ColorTriplet::Zero();
		float msiS = 0.0f;

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

			if (outL.PDF_S <= PR_EPSILON) {
				return LiL;
			} else {
				// Trace shadow ray
				Ray shadow			= spt.Ray.next(spt.P, outL.Outgoing);
				ShadowHit shadowHit = session.traceShadowRay(shadow);
				if (shadowHit.Successful)
					return LiL;

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

		if (!allowMSI || LiL.isZero())
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
		else
			return LiL * msiL + LiS * msiS;
	}

	void scatterLight(RenderTileSession& session, LightPath& path,
					  const ShadingPoint& spt,
					  IMaterial* material)
	{
		PR_PROFILE_THIS;

		constexpr int MIN_DEPTH			= 4; //Minimum level of depth after which russian roulette will apply
		constexpr float DEPTH_DROP_RATE = 0.90f;

		const float scatProb  = std::min<float>(1.0f, pow(DEPTH_DROP_RATE, (int)spt.Ray.IterationDepth - MIN_DEPTH));
		const size_t maxDepth = session.tile()->context()->settings().maxRayDepth;

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
			&& spt.Ray.IterationDepth + 1 < maxDepth
			&& session.tile()->random().getFloat() <= scatProb) {
			Ray next = spt.Ray.next(spt.P, out.Outgoing);
			next.Weight *= out.Weight / (scatProb * out.PDF_S);

			if (!next.Weight.isZero()) {
				path.addToken(LightPathToken(out.Type));

				GeometryPoint npt;
				IEntity* nentity;
				IMaterial* nmaterial;
				if (session.traceBounceRay(next, npt, nentity, nmaterial)) {
					/*ShadingPoint snpt;
					snpt.setByIdentity(next, npt);
					const float NdotV = std::abs(snpt.NdotV);
					next.Weight *= NdotV/snpt.Depth2;
					if (!next.Weight.isZero())*/
						eval(session, path, next, npt, nentity, nmaterial);
				} else {
					session.tile()->statistics().addBackgroundHitCount();

					for (auto light : session.tile()->context()->scene()->infiniteLights()) {
						if (light->hasDeltaDistribution())
							continue;

						// Only the ray is fixed (Should be handled better!)
						InfiniteLightEvalInput lin;
						lin.Point.Ray   = next;
						lin.Point.Flags = SPF_Background;
						InfiniteLightEvalOutput lout;
						light->eval(lin, lout, session);

						in.Point.Radiance = lout.Weight;
						session.pushFragment(in.Point, path.concated(LightPathToken(ST_CAMERA, SE_NONE)));
					}
				}
			}
		}
	}

	ColorTriplet directLight(RenderTileSession& session, const ShadingPoint& spt,
							 LightPathToken& token,
							 IMaterial* material)
	{
		PR_PROFILE_THIS;

		ColorTriplet Li = ColorTriplet::Zero();

		// Pick light and point
		float pdfA;
		GeometryPoint lightPt;
		IEntity* light = session.pickRandomLight(spt.N, lightPt, pdfA);
		if (!light)
			return Li;
		IEmission* ems = session.getEmission(lightPt.EmissionID);
		if (!ems) {
			session.pushFeedbackFragment(spt.Ray, OF_MissingEmission);
			return Li;
		}

		// (1) Sample light
		{
			Vector3f L = (lightPt.P - spt.P);
			//const float sqrD = L.squaredNorm();
			L.normalize();
			//const float cosO = std::max(0.0f, -L.dot(lightPt.N));
			const float cosI = L.dot(spt.N);

			if (std::abs(cosI) < PR_EPSILON
				//|| cosO < PR_EPSILON
				//|| sqrD < PR_EPSILON
				|| pdfA < PR_EPSILON)
				return Li;

			// Trace shadow ray
			Ray shadow			= spt.Ray.next(spt.P, L);
			ShadowHit shadowHit = session.traceShadowRay(shadow);

			if (!shadowHit.Successful
				|| shadowHit.EntityID != light->id()) {
				return Li;
			} else {
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

				float msi = mMSIEnabled ? MSI(1, pdfA, 1, out.PDF_S) : 1.0f;
				Li		  = outL.Weight * out.Weight * msi / pdfA;
			}
		}

		if (!mMSIEnabled || Li.isZero())
			return Li;

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

						float msi = MSI(1, 1 / light->surfaceArea(), 1, outS.PDF_S);
						Li += outL.Weight * outS.Weight * msi / outS.PDF_S;
					}
				}
			}
		}

		return Li;
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
		spt.Radiance = ColorTriplet::Ones();

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

				const float prevGEOM = /*std::abs(spt.NdotV) * std::abs(spt.Ray.NdotL)*/ 1 /* spt.Depth2*/;
				spt.Radiance		 = outL.Weight * prevGEOM;

				if (!spt.Radiance.isZero())
					session.pushFragment(spt, path.concated(
												  LightPathToken(ST_EMISSIVE, SE_NONE)));
			}
		}

		if (!material)
			return;

		// Scatter Light
		scatterLight(session, path, spt, material);

		if (!material->hasDeltaDistribution()) {
			// Direct Light
			const float factor = 1.0f / mLightSampleCount;
			for (size_t i = 0; i < mLightSampleCount; ++i) {
				LightPathToken token;
				spt.Radiance = factor * directLight(session, spt, token, material);

				if (!spt.Radiance.isZero()) {
					LightPath new_path = path;
					new_path.concat(token);
					new_path.concat(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushFragment(spt, new_path);
				}
			}

			// Infinite Lights
			for (auto light : session.tile()->context()->scene()->infiniteLights()) {
				LightPathToken token;
				spt.Radiance = infiniteLight(session, spt, token,
											 light.get(), material);

				if (!spt.Radiance.isZero()) {
					LightPath new_path = path;
					new_path.concat(token);
					new_path.concat(LightPathToken(ST_BACKGROUND, SE_NONE));
					session.pushFragment(spt, new_path);
				}
			}
		}
	}

	// Per thread
	void onPass(RenderTileSession& session, uint32) override
	{
		PR_PROFILE_THIS;

		const size_t maxPathSize = session.tile()->context()->settings().maxRayDepth + 2;

		LightPath cb = LightPath::createCB();
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

						in.Point.Radiance = out.Weight;
						session.pushFragment(in.Point, cb);
					}
				},
				[&](const HitEntry& /*entry*/,
					const Ray& ray, const GeometryPoint& pt,
					IEntity* entity, IMaterial* material) {
					LightPath path(maxPathSize);
					path.addToken(LightPathToken(ST_CAMERA, SE_NONE));
					eval(session, path, ray, pt, entity, material);
				});
		}
	}

	RenderStatus status() const override
	{
		return RenderStatus();
	}

	inline void enableMSI(bool b) { mMSIEnabled = b; }

private:
	std::vector<std::unique_ptr<LightPathBuffer>> mLPBs;

	size_t mLightSampleCount;
	bool mMSIEnabled;
};

class IntDirectFactory : public IIntegratorFactory {
public:
	std::shared_ptr<IIntegrator> create(uint32, uint32, const Environment& env) override
	{
		const Registry& reg = env.registry();

		size_t lightsamples = (size_t)reg.getByGroup<uint32>(RG_INTEGRATOR, "direct/light/sample_count", 4);

		auto obj = std::make_shared<IntDirect>(lightsamples);
		obj->enableMSI(reg.getByGroup<bool>(RG_INTEGRATOR, "direct/msi/enabled", true));
		return obj;
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

PR_PLUGIN_INIT(PR::IntDirectFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)