#include "Light.h"

#include "LightSampler.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "infinitelight/IInfiniteLight.h"
#include "math/Sampling.h"
#include "renderer/RenderTileSession.h"
#include "spectral/ISpectralMapper.h"

namespace PR {
Light::Light(uint32 light_id, IInfiniteLight* infLight, float relContribution, const LightSampler* sampler)
	: mID(light_id)
	, mEntity(infLight)
	, mEmission(nullptr)
	, mRelativeContribution(relContribution)
	, mSampler(sampler)
{
	PR_ASSERT(mSampler, "Expected valid sampler");
}

Light::Light(uint32 light_id, IEntity* entity, IEmission* emission, float relContribution, const LightSampler* sampler)
	: mID(light_id)
	, mEntity(entity)
	, mEmission(emission)
	, mRelativeContribution(relContribution)
	, mSampler(sampler)
{
	PR_ASSERT(mSampler, "Expected valid sampler");
}

std::string Light::name() const
{
	if (isInfinite())
		return reinterpret_cast<IInfiniteLight*>(mEntity)->name();
	else
		return reinterpret_cast<IEntity*>(mEntity)->name();
}

bool Light::hasDeltaDistribution() const { return isInfinite() && reinterpret_cast<IInfiniteLight*>(mEntity)->hasDeltaDistribution(); }

SpectralBlob Light::averagePower(const SpectralBlob& wavelengths) const
{
	if (isInfinite()) {
		IInfiniteLight* infL = reinterpret_cast<IInfiniteLight*>(mEntity);
		return infL->power(wavelengths);
	} else {
		PR_ASSERT(mEmission, "Invalid light given!");
		return mEmission->power(wavelengths);
	}
}

SpectralRange Light::spectralRange() const
{
	if (isInfinite()) {
		IInfiniteLight* infL = reinterpret_cast<IInfiniteLight*>(mEntity);
		return infL->spectralRange();
	} else {
		PR_ASSERT(mEmission, "Invalid light given!");
		return mEmission->spectralRange();
	}
}

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
	} else if (in.Point) {
		IEntity* hitEntity = session.getEntity(in.Point->Surface.Geometry.EntityID);
		IEntity* ent	   = reinterpret_cast<IEntity*>(mEntity);
		// TODO: Why check??
		if (hitEntity != ent) { // No real hit, giveup.
			out.Radiance   = SpectralBlob::Zero();
			out.PDF.Value  = 0;
			out.PDF.IsArea = false;
			return;
		}

		EmissionEvalInput eein;
		eein.Context		= EmissionEvalContext::fromIP(*in.Point, -in.Ray.Direction);
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
	out.WavelengthNM   = in.WavelengthNM;
	out.Wavelength_PDF = 1;

	if (isInfinite()) {
		if (in.SampleWavelength) {
			SpectralSampleInput ssin(session.tile()->random(RandomSlot::Light));
			ssin.Light	 = this;
			ssin.Purpose = SpectralSamplePurpose::Light;

			SpectralSampleOutput ssout;
			mSampler->wavelengthSampler()->sample(ssin, ssout);
			out.WavelengthNM   = ssout.WavelengthNM;
			out.Wavelength_PDF = ssout.PDF;
		}

		IInfiniteLight* infL = reinterpret_cast<IInfiniteLight*>(mEntity);

		if (in.SamplePosition) {
			InfiniteLightSamplePosDirInput ilsin;
			ilsin.DirectionRND = in.RND.get2D();
			ilsin.PositionRND  = in.RND.get2D();
			ilsin.WavelengthNM = out.WavelengthNM;
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
			ilsin.DirectionRND = in.RND.get2D();
			ilsin.WavelengthNM = out.WavelengthNM;

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
		const EntitySamplePoint pp = in.SamplingInfo ? ent->sampleParameterPoint(*in.SamplingInfo, in.RND.get2D())
													 : ent->sampleParameterPoint(in.RND.get2D());
		const EntityGeometryQueryPoint qp = pp.toQueryPoint(Vector3f::Zero());

		GeometryPoint gp;
		ent->provideGeometryPoint(qp, gp);

		if (in.SampleWavelength) {
			SpectralSampleInput ssin(session.tile()->random(RandomSlot::Light));
			ssin.Light	  = this;
			ssin.Position = pp.Position;
			ssin.Purpose  = SpectralSamplePurpose::Light;

			SpectralSampleOutput ssout;
			mSampler->wavelengthSampler()->sample(ssin, ssout);
			out.WavelengthNM   = ssout.WavelengthNM;
			out.Wavelength_PDF = ssout.PDF;
		}

		ShadingContext sctx;
		sctx.Face		  = gp.PrimitiveID;
		sctx.dUV		  = gp.dUV;
		sctx.UV			  = gp.UV;
		sctx.ThreadIndex  = session.threadID();
		sctx.WavelengthNM = out.WavelengthNM;

		EmissionSampleContext ectx;
		ectx.P			 = pp.Position;
		ectx.PrimitiveID = gp.PrimitiveID;
		ectx.RayFlags	 = RayFlag::Light;
		ectx.UV			 = gp.UV;

		if (!in.Point) { // Sample a complete new direction
			EmissionSampleInput esin(in.RND);
			esin.ShadingContext = sctx;
			esin.Context		= ectx;

			EmissionSampleOutput esout;
			mEmission->sample(esin, esout, session);

			out.Outgoing		= -esout.L;
			out.Direction_PDF_S = esout.PDF_S[0];
		} else { // Sample only wavelength if necessary
			out.Outgoing		= (pp.Position - in.Point->P).normalized();
			out.Direction_PDF_S = 1;
		}

		// Evaluate to get the radiance
		EmissionEvalInput eein;

		eein.Context = ectx.expand(-out.Outgoing);

		eein.ShadingContext				 = sctx;
		eein.ShadingContext.WavelengthNM = out.WavelengthNM;

		EmissionEvalOutput eeout;
		mEmission->eval(eein, eeout, session);

		out.Radiance			= eeout.Radiance;
		out.Position_PDF.Value	= pp.PDF_A;
		out.Position_PDF.IsArea = true;
		out.LightPosition		= pp.Position;
		out.CosLight			= -out.Outgoing.dot(gp.N);
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

float Light::pdfDirection(const Vector3f& dir, float cosLight) const
{ // TODO Make this emission and inf light specific
	if (isInfinite()) {
		IInfiniteLight* infL = reinterpret_cast<IInfiniteLight*>(mEntity);
		if (infL->hasDeltaDistribution())
			return 1;
		else
			return Sampling::cos_hemi_pdf(std::abs(dir[2]));
	} else {
		return Sampling::cos_hemi_pdf(std::abs(cosLight));
	}
}
} // namespace PR