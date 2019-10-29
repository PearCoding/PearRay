#include "Environment.h"
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

#define MSI(n1, p1, n2, p2) IS::balance_term((n1), (p1), (n2), (p2))

namespace PR {
class IntDirect : public IIntegrator {
public:
	IntDirect(size_t lightSamples)
		: IIntegrator()
		, mLightSampleCount(lightSamples)
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

	float infiniteLight(RenderTileSession& session, const ShadingPoint& spt, LightPathToken& token,
						IInfiniteLight* infLight, IMaterial* material)
	{
		InfiniteLightSampleInput inL;
		inL.Point = spt;
		inL.RND   = session.tile()->random().get2D();
		InfiniteLightSampleOutput outL;
		infLight->sample(inL, outL, session);

		if (std::isinf(outL.PDF_S))
			outL.PDF_S = 1;

		const float cosI = outL.Outgoing.dot(spt.N);
		if (std::abs(cosI) < PR_EPSILON
			|| outL.Weight < PR_EPSILON
			|| outL.PDF_S < PR_EPSILON) {
			return 0;
		} else {
			// Trace shadow ray
			Ray shadow			= spt.Ray.next(spt.P, outL.Outgoing);
			ShadowHit shadowHit = session.traceShadowRay(shadow);
			if (shadowHit.Successful)
				return 0;

			// Evaluate surface
			MaterialEvalInput in;
			in.Point	= spt;
			in.Outgoing = outL.Outgoing;
			in.NdotL	= cosI;
			MaterialEvalOutput out;
			material->eval(in, out, session);
			token = LightPathToken(out.Type);

			return outL.Weight * out.Weight * std::abs(cosI) / outL.PDF_S;
		}
	}

	void scatterLight(RenderTileSession& session, LightPathBuffer& lpb,
					  const HitEntry& entry, const ShadingPoint& spt,
					  IMaterial* material, bool& scattered, bool& isDelta)
	{
		constexpr int MIN_DEPTH			= 4; //Minimum level of depth which russian roulette will apply
		constexpr float DEPTH_DROP_RATE = 0.90f;

		const float scatProb  = std::min<float>(1.0f, pow(DEPTH_DROP_RATE, (int)spt.Ray.Depth - MIN_DEPTH));
		const size_t maxDepth = session.tile()->context()->settings().maxRayDepth;
		const float emsArea   = session.tile()->context()->emissiveSurfaceArea();

		scattered = false;
		isDelta   = false;

		// Sample BxDF
		MaterialSampleInput in;
		in.Point = spt;
		in.RND   = session.tile()->random().get2D();
		MaterialSampleOutput out;
		material->sample(in, out, session);

		const float cosI = std::abs(out.Outgoing.dot(spt.N));
		if (std::isinf(out.PDF_S_Forward)) {
			isDelta			  = true;
			out.PDF_S_Forward = 1;
		}

		if (cosI > PR_EPSILON
			&& spt.Ray.Depth + 1 < maxDepth
			&& session.tile()->random().getFloat() <= scatProb) {
			Ray next = spt.Ray.next(spt.P, out.Outgoing);

			// This is only a bad approximation
			float msi = isDelta ? 1.0f : MSI(mLightSampleCount, 1 / emsArea, 1, out.PDF_S_Forward);
			next.Weight *= msi * out.Weight * cosI / (scatProb * out.PDF_S_Forward);

			if (next.Weight > PR_EPSILON) {
				LightPathBufferEntry pathEntry;
				switch (out.Type) {
				case MST_DiffuseReflection:
					pathEntry.Flags = LPBEF_EVENT_DIFFUSE | LPBEF_SCATTER_REFLECTION;
					break;
				case MST_DiffuseTransmission:
					pathEntry.Flags = LPBEF_EVENT_DIFFUSE;
					break;
				case MST_SpecularReflection:
					pathEntry.Flags = LPBEF_SCATTER_REFLECTION;
					break;
				case MST_SpecularTransmission:
					pathEntry.Flags = 0;
					break;
				}
				pathEntry.LabelIndex = 0; // TODO
				lpb.add(entry.RayID, pathEntry);
				session.sendRay(entry.SessionRayID, next);
				scattered = true;
			}
		}
	}

	float directLight(RenderTileSession& session, const ShadingPoint& spt, LightPathToken& token,
					  IMaterial* material)
	{
		// Pick light and point
		float pdfA;
		GeometryPoint lightPt;
		IEntity* light = session.pickRandomLight(lightPt, pdfA);
		if (!light)
			return 0;
		IEmission* ems = session.getEmission(lightPt.EmissionID);
		if (!ems) {
			session.pushFeedbackFragment(spt.Ray, OF_MissingEmission);
			return 0;
		}

		Vector3f L		 = (lightPt.P - spt.P);
		const float sqrD = L.squaredNorm();
		L.normalize();
		const float cosO = std::max(0.0f, -L.dot(lightPt.N));
		const float cosI = L.dot(spt.N);
		float pdfS		 = IS::toSolidAngle(pdfA, sqrD, cosO);

		// Trace shadow ray
		Ray shadow			= spt.Ray.next(spt.P, L);
		ShadowHit shadowHit = session.traceShadowRay(shadow);
		if (pdfS < PR_EPSILON
			|| std::abs(cosI) < PR_EPSILON
			|| !shadowHit.Successful
			|| shadowHit.EntityID != light->id()) {
			return 0;
		} else {
			// Evaluate light
			LightEvalInput inL;
			inL.Entity = light;
			inL.Point.setByIdentity(shadow, lightPt); // Todo??
			LightEvalOutput outL;
			ems->eval(inL, outL, session);

			// Evaluate surface
			MaterialEvalInput in;
			in.Point	= spt;
			in.Outgoing = L;
			in.NdotL	= cosI;
			MaterialEvalOutput out;
			material->eval(in, out, session);
			token = LightPathToken(out.Type);

			float msi_weight = MSI(mLightSampleCount, pdfS, 1, out.PDF_S_Forward);

			return msi_weight * outL.Weight * out.Weight * std::abs(cosI) / pdfS;
		}
	}

	bool perHit(RenderTileSession& session, LightPathBuffer& lpb,
				const HitEntry& entry,
				const Ray& ray, const GeometryPoint& pt,
				IEntity* entity, IMaterial* material)
	{
		session.tile()->statistics().addEntityHitCount();

		if (entity->isLight()) // Lights are directly traced
			return false;
		if (!material)
			return false;

		LightPath currentPath = lpb.getPath(entry.RayID);

		ShadingPoint spt;
		spt.setByIdentity(ray, pt);
		spt.EntityID = entity->id();
		spt.Radiance = 1;

		// Scatter Light
		bool scattered = false;
		bool isDelta   = false;
		scatterLight(session, lpb, entry, spt, material, scattered, isDelta);

		// Direct Light
		if (!isDelta) {
			const float factor = 1.0f / mLightSampleCount;
			for (size_t i = 0; i < mLightSampleCount; ++i) {
				LightPathToken token;
				spt.Radiance = factor * directLight(session, spt, token, material);

				if (spt.Radiance > PR_EPSILON) {
					LightPath path = currentPath;
					path.concat(token);
					path.concat(LightPathToken(ST_EMISSIVE, SE_NONE));
					session.pushFragment(spt, path);
				}
			}
		}

		// Infinite Lights
		if (!isDelta) {
			for (auto light : session.tile()->context()->scene()->infiniteLights()) {
				if (light->isBackground())
					continue;

				LightPathToken token;
				spt.Radiance = infiniteLight(session, spt, token,
											 light.get(), material);

				if (spt.Radiance > PR_EPSILON) {
					LightPath path = currentPath;
					path.concat(token);
					path.concat(LightPathToken(ST_BACKGROUND, SE_NONE));
					session.pushFragment(spt, path);
				}
			}
		}

		return scattered;
	}

	// Per thread
	void onPass(RenderTileSession& session, uint32) override
	{
		while (session.handleCameraRays()) {
			mLPBs.at(session.threadID())->reset();
			float rerun = true;

			while (rerun) {
				rerun = false;
				session.handleHits(
					[&](size_t ray_id, const Ray& ray) {
						session.tile()->statistics().addBackgroundHitCount();

						LightPath currentPath = mLPBs.at(session.threadID())->getPath(ray_id);
						for (auto light : session.tile()->context()->scene()->infiniteLights()) {
							if (!light->isBackground())
								continue;

							// Only the ray is fixed (Should be handled better!)
							InfiniteLightEvalInput in;
							in.Point.Ray   = ray;
							in.Point.Flags = SPF_Background;
							InfiniteLightEvalOutput out;
							light->eval(in, out, session);

							in.Point.Radiance = out.Weight;
							session.pushFragment(in.Point,
												 currentPath.concated(
													 LightPathToken(ST_BACKGROUND, SE_NONE)));
						}
					},
					[&](const HitEntry& entry,
						const Ray& ray, const GeometryPoint& pt,
						IEntity* entity, IMaterial* material) {
						float scattered = perHit(session, *mLPBs.at(session.threadID()).get(),
												 entry, ray, pt, entity, material);
						if (scattered)
							rerun = true;
					});
			}
		}
	}

	RenderStatus status() const override
	{
		return RenderStatus();
	}

private:
	std::vector<std::unique_ptr<LightPathBuffer>> mLPBs;

	size_t mLightSampleCount;
};

class IntDirectFactory : public IIntegratorFactory {
public:
	std::shared_ptr<IIntegrator> create(uint32, uint32, const Environment& env) override
	{
		const Registry& reg = env.registry();

		size_t lightsamples = (size_t)reg.getByGroup<uint32>(RG_INTEGRATOR, "direct/light/sample_count", 4);

		return std::make_shared<IntDirect>(lightsamples);
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

PR_PLUGIN_INIT(PR::IntDirectFactory, "int_direct", PR_PLUGIN_VERSION)