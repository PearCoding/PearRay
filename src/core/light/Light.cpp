#include "Light.h"

#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "math/Sampling.h"
#include "renderer/RenderTileSession.h"

namespace PR {
Light::Light(IInfiniteLight* infLight, float relContribution)
	: mEntity(infLight)
	, mEmission(nullptr)
	, mRelativeContribution(relContribution)
{
}

Light::Light(IEntity* entity, IEmission* emission, float relContribution)
	: mEntity(entity)
	, mEmission(emission)
	, mRelativeContribution(relContribution)
{
}

std::string Light::name() const
{
	if (isInfinite())
		return reinterpret_cast<IInfiniteLight*>(mEntity)->name();
	else
		return reinterpret_cast<IEntity*>(mEntity)->name();
}

uint32 Light::id() const
{
	if (isInfinite())
		return reinterpret_cast<IInfiniteLight*>(mEntity)->id();
	else
		return reinterpret_cast<IEntity*>(mEntity)->id();
}

uint32 Light::entityID() const
{
	if (isInfinite())
		return PR_INVALID_ID;
	else
		return reinterpret_cast<IEntity*>(mEntity)->id();
}

uint32 Light::infiniteLightID() const
{
	if (isInfinite())
		return reinterpret_cast<IInfiniteLight*>(mEntity)->id();
	else
		return PR_INVALID_ID;
}

bool Light::hasDeltaDistribution() const { return isInfinite() && reinterpret_cast<IInfiniteLight*>(mEntity)->hasDeltaDistribution(); }

void Light::eval(const LightEvalInput& in, LightEvalOutput& out, const RenderTileSession& session) const
{
	if (isInfinite()) {
		IInfiniteLight* infL = reinterpret_cast<IInfiniteLight*>(mEntity);

		InfiniteLightEvalInput ilein;
		ilein.WavelengthNM	 = in.Ray.WavelengthNM;
		ilein.Direction		 = in.Ray.Direction;
		ilein.IterationDepth = in.Ray.IterationDepth;
		InfiniteLightEvalOutput ileout;
		infL->eval(ilein, ileout, session);

		out.Radiance   = ileout.Radiance;
		out.PDF.Value  = ileout.Direction_PDF_S;
		out.PDF.IsArea = false;
	} else if (in.Point
			   && in.Point->Surface.Geometry.EntityID == reinterpret_cast<IEntity*>(mEntity)->id()) {
		IEntity* ent = reinterpret_cast<IEntity*>(mEntity);

		EmissionEvalInput eein;
		eein.Entity			= ent;
		eein.ShadingContext = ShadingContext::fromIP(session.threadID(), *in.Point);

		EmissionEvalOutput eeout;
		mEmission->eval(eein, eeout, session);

		const float entPDF_A = ent->sampleParameterPointPDF();
		out.Radiance		 = eeout.Radiance;
		out.PDF.Value		 = entPDF_A * Sampling::cos_hemi_pdf(in.Point->Surface.NdotV);
		out.PDF.IsArea		 = true;
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

		if (in.SamplePosition) {
			InfiniteLightSamplePosDirInput ilsin;
			ilsin.DirectionRND = Vector2f(in.RND(0), in.RND(1));
			ilsin.PositionRND  = Vector2f(in.RND(2), in.RND(3));
			ilsin.WavelengthNM = in.WavelengthNM;
			ilsin.Point		   = in.Point;

			InfiniteLightSamplePosDirOutput ilsout;
			infL->samplePosDir(ilsin, ilsout, session);

			out.Radiance			= ilsout.Radiance;
			out.Position_PDF.Value	= ilsout.Position_PDF_A;
			out.Position_PDF.IsArea = true;
			out.Direction_PDF_S		= ilsout.Direction_PDF_S;
			out.Outgoing			= ilsout.Outgoing;
			out.LightPosition		= ilsout.LightPosition;
		} else {
			InfiniteLightSampleDirInput ilsin;
			ilsin.DirectionRND = Vector2f(in.RND(0), in.RND(1));
			ilsin.WavelengthNM = in.WavelengthNM;

			InfiniteLightSampleDirOutput ilsout;
			infL->sampleDir(ilsin, ilsout, session);

			out.Radiance			= ilsout.Radiance;
			out.Position_PDF.Value	= 0;
			out.Position_PDF.IsArea = false;
			out.Direction_PDF_S		= ilsout.Direction_PDF_S;
			out.Outgoing			= ilsout.Outgoing;
			out.LightPosition		= Vector3f::Zero();
		}
		out.CosLight = 1;
	} else {
		IEntity* ent			   = reinterpret_cast<IEntity*>(mEntity);
		const EntitySamplePoint pp = in.SamplingInfo ? ent->sampleParameterPoint(*in.SamplingInfo, Vector2f(in.RND[0], in.RND[1]))
													 : ent->sampleParameterPoint(Vector2f(in.RND[0], in.RND[1]));

		EntityGeometryQueryPoint qp;
		qp.Position	   = pp.Position;
		qp.UV		   = pp.UV;
		qp.PrimitiveID = pp.PrimitiveID;
		qp.View		   = Vector3f::Zero();

		GeometryPoint gp;
		ent->provideGeometryPoint(qp, gp);

		EmissionEvalInput eein;
		eein.Entity						 = ent;
		eein.ShadingContext.Face		 = gp.PrimitiveID;
		eein.ShadingContext.dUV			 = gp.dUV;
		eein.ShadingContext.UV			 = gp.UV;
		eein.ShadingContext.ThreadIndex	 = session.threadID();
		eein.ShadingContext.WavelengthNM = in.WavelengthNM;

		EmissionEvalOutput eeout;
		mEmission->eval(eein, eeout, session);

		out.Radiance			= eeout.Radiance;
		out.Position_PDF.Value	= pp.PDF_A;
		out.Position_PDF.IsArea = true;
		out.LightPosition		= pp.Position;

		if (!in.Point) {
			// Randomly sample direction from emissive material (TODO: Should be abstracted -> Use Emission)
			const Vector3f local_dir = Sampling::cos_hemi(in.RND[2], in.RND[3]);
			const Vector3f dir		 = -Tangent::fromTangentSpace(gp.N, gp.Nx, gp.Ny, local_dir);

			out.Direction_PDF_S = Sampling::cos_hemi_pdf(local_dir(2));
			out.Outgoing		= dir;
			out.CosLight		= local_dir(2);
		} else { // Connect to given surface
			out.Outgoing		= (pp.Position - in.Point->P).normalized();
			out.Direction_PDF_S = 1;
			out.CosLight		= -out.Outgoing.dot(gp.N);
		}
	}
}

LightPDF Light::pdfPosition(const Vector3f& posOnLight, const EntitySamplingInfo* info) const
{ // TODO Make this emission and inf light specific
	if (isInfinite()) {
		return LightPDF{ PR_INV_2_PI, false }; // TODO
	} else {
		IEntity* ent   = reinterpret_cast<IEntity*>(mEntity);
		const auto pdf = info ? ent->sampleParameterPointPDF(posOnLight, *info) : ent->sampleParameterPointPDF();
		return LightPDF{ pdf, true };
	}
}

float Light::pdfDirection(const Vector3f& dir, const EntitySamplingInfo* info) const
{ // TODO Make this emission and inf light specific
	if (isInfinite()) {
		IInfiniteLight* infL = reinterpret_cast<IInfiniteLight*>(mEntity);
		if (infL->hasDeltaDistribution())
			return 1;
		else
			return Sampling::cos_hemi_pdf(dir[2]);
	} else {
		if (!info)
			return 0.0f;
		else
			return Sampling::cos_hemi_pdf(std::abs(info->Normal.dot(dir)));
	}
}
} // namespace PR