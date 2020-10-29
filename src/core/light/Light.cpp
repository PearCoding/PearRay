#include "Light.h"

#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "math/Sampling.h"
#include "renderer/RenderTileSession.h"

namespace PR {
Light::Light(IInfiniteLight* infLight)
	: mEntity(infLight)
	, mEmission(nullptr)
{
}

Light::Light(IEntity* entity, IEmission* emission)
	: mEntity(entity)
	, mEmission(emission)
{
}

bool Light::hasDeltaDistribution() const { return isInfinite() && reinterpret_cast<IInfiniteLight*>(mEntity)->hasDeltaDistribution(); }

void Light::eval(const LightEvalInput& in, LightEvalOutput& out, const RenderTileSession& session) const
{
	if (isInfinite()) {
		IInfiniteLight* infL = reinterpret_cast<IInfiniteLight*>(mEntity);

		InfiniteLightEvalInput ilein;
		ilein.Point = in.Point;
		ilein.Ray	= in.Ray;
		InfiniteLightEvalOutput ileout;
		infL->eval(ilein, ileout, session);

		out.Radiance   = ileout.Radiance;
		out.PDF.Value  = ileout.PDF_S;
		out.PDF.IsArea = false;
	} else if (in.Point
			   && in.Point->Surface.Geometry.EntityID == reinterpret_cast<IEntity*>(mEntity)->id()) {
		IEntity* ent = reinterpret_cast<IEntity*>(mEntity);

		EmissionEvalInput eein;
		eein.Entity			= ent;
		eein.ShadingContext = ShadingContext::fromIP(session.threadID(), *in.Point);

		EmissionEvalOutput eeout;
		mEmission->eval(eein, eeout, session);

		const auto entPDF = ent->sampleParameterPointPDF();
		out.Radiance	  = eeout.Weight;
		out.PDF.Value	  = entPDF.Value * Sampling::cos_hemi_pdf(in.Point->Surface.NdotV);
		out.PDF.IsArea	  = entPDF.IsArea;
	} else { // No point information, giveup
		out.Radiance   = SpectralBlob::Zero();
		out.PDF.Value  = 0;
		out.PDF.IsArea = false;
	}
}

void Light::sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const
{
	if (isInfinite()) {
		IInfiniteLight* infL = reinterpret_cast<IInfiniteLight*>(mEntity);

		InfiniteLightSampleInput ilsin;
		ilsin.RND		   = Vector2f(in.RND[0], in.RND[1]);
		ilsin.WavelengthNM = in.WavelengthNM;

		InfiniteLightSampleOutput ilsout;
		infL->sample(ilsin, ilsout, session);

		out.Radiance   = ilsout.Radiance;
		out.PDF.Value  = ilsout.PDF_S;
		out.PDF.IsArea = false;
		out.Outgoing   = ilsout.Outgoing;
	} else {
		IEntity* ent  = reinterpret_cast<IEntity*>(mEntity);
		const auto pp = ent->sampleParameterPoint(Vector2f(in.RND[0], in.RND[1]));

		EntityGeometryQueryPoint qp;
		qp.Position	   = pp.Position;
		qp.UV		   = pp.UV;
		qp.PrimitiveID = pp.PrimitiveID;
		qp.View		   = Vector3f::Zero();

		GeometryPoint gp;
		ent->provideGeometryPoint(qp, gp);

		// Randomly sample direction from emissive material (TODO: Should be abstracted -> Use Emission)
		const Vector3f local_dir = Sampling::cos_hemi(in.RND[2], in.RND[3]);
		const Vector3f dir		 = Tangent::fromTangentSpace(gp.N, gp.Nx, gp.Ny, local_dir);

		EmissionEvalInput eein;
		eein.Entity						 = ent;
		eein.ShadingContext.Face		 = gp.PrimitiveID;
		eein.ShadingContext.dUV			 = gp.dUV;
		eein.ShadingContext.UV			 = gp.UV;
		eein.ShadingContext.ThreadIndex	 = session.threadID();
		eein.ShadingContext.WavelengthNM = in.WavelengthNM;

		EmissionEvalOutput eeout;
		mEmission->eval(eein, eeout, session);

		out.Radiance   = eeout.Weight;
		out.PDF.Value  = pp.PDF.Value * Sampling::cos_hemi_pdf(local_dir(2));
		out.PDF.IsArea = pp.PDF.IsArea;
		out.Outgoing   = dir;
	}
}

} // namespace PR