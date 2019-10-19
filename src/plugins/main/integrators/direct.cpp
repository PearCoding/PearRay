#include "buffer/Feedback.h"
#include "emission/IEmission.h"
#include "integrator/IIntegrator.h"
#include "integrator/IIntegratorFactory.h"
#include "material/IMaterial.h"
#include "math/MSI.h"
#include "path/LightPath.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderTile.h"
#include "renderer/RenderTileSession.h"
#include "shader/ShadingPoint.h"

#include "Logger.h"

namespace PR {
class IntDirect : public IIntegrator {
public:
	IntDirect()
		: IIntegrator()
	{
	}

	virtual ~IntDirect() = default;

	void init(RenderContext*) override
	{
	}

	void onStart() override
	{
	}

	void onNextPass(uint32, bool&) override
	{
	}

	void onEnd() override
	{
	}

	bool needNextPass(uint32 i) const override
	{
		return i == 0;
	}

	void perHit(RenderTileSession& session, const HitEntry&,
				const Ray& ray, const GeometryPoint& pt,
				IEntity*, IMaterial* material)
	{
		LightPath stdPath = LightPath::createCDL(1);
		session.tile()->statistics().addEntityHitCount();

		ShadingPoint spt;
		spt.setByIdentity(ray, pt);
		spt.Radiance = 1;

		// Pick light and point
		float pdfA;
		GeometryPoint lightPt;
		IEntity* light = session.pickRandomLight(lightPt, pdfA);
		if (!light)
			return;
		IEmission* ems = session.getEmission(lightPt.EmissionID);
		if (!ems) {
			session.pushFeedbackFragment(ray, OF_MissingMaterial);
			return;
		}

		Vector3f L		 = (lightPt.P - spt.P);
		const float sqrD = L.squaredNorm();
		L.normalize();
		const float cosO = std::abs(L.dot(lightPt.N));
		const float cosI = std::max(0.0f, L.dot(spt.N));
		const float pdfS = MSI::toSolidAngle(pdfA, sqrD, cosO);

		// Trace shadow ray
		Ray shadow			= ray.next(spt.P, L);
		ShadowHit shadowHit = session.traceShadowRay(shadow);
		if (pdfS < PR_EPSILON
			|| cosI < PR_EPSILON
			|| !shadowHit.Successful
			|| shadowHit.EntityID != light->id()) {
			spt.Radiance = 0;
		} else {
			// Evaluate light
			LightEvalInput inL;
			inL.Point.setByIdentity(shadow, lightPt); // Todo??
			LightEvalOutput outL;
			ems->eval(inL, outL, session);
			spt.Radiance *= outL.Weight;

			// Evaluate surface
			MaterialEvalInput in;
			in.Point	= spt;
			in.Outgoing = L;

			MaterialEvalOutput out;
			material->eval(in, out, session);

			spt.Radiance *= out.Weight * cosI / pdfS;
		}

		session.pushFragment(spt, stdPath);
	}

	// Per thread
	void onPass(RenderTileSession& session, uint32) override
	{
		while (session.handleCameraRays()) {
			session.handleHits([&](const HitEntry& entry,
								   const Ray& ray, const GeometryPoint& pt,
								   IEntity* entity, IMaterial* material) {
				perHit(session, entry, ray, pt, entity, material);
			});
		}
	}

	RenderStatus status() const override
	{
		return RenderStatus();
	}

private:
	std::vector<LightPath> mLightPaths;
};

class IntDirectFactory : public IIntegratorFactory {
public:
	std::shared_ptr<IIntegrator> create(uint32, uint32, const Environment& env) override
	{
		return std::make_shared<IntDirect>();
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