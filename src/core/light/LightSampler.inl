// IWYU pragma: private, include "light/LightSampler.h"

namespace PR {
inline const Light* LightSampler::sample(float rnd, float& pdf) const
{
	PR_ASSERT(mSelector, "Expected initialized light sampler");
	const size_t id = mSelector->sampleDiscrete(rnd, pdf);
	return id < mLights.size() ? mLights[id].get() : nullptr;
}

inline const Light* LightSampler::sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const
{
	PR_ASSERT(mSelector, "Expected initialized light sampler");
	float pdf, rem;
	const size_t id = mSelector->sampleDiscrete(in.RND[0], pdf, &rem);
	if (id < mLights.size()) {
		const Light* l = mLights[id].get();

		LightSampleInput in2 = in;
		in2.RND[0]			 = rem;
		l->sample(in2, out, session);

		out.PDF.Value *= pdf;
		return l;
	} else {
		return nullptr;
	}
}

LightPDF LightSampler::pdf(const IEntity* entity) const
{
	if (!entity->hasEmission() || mLightEntityMap.count(entity) == 0)
		return LightPDF{ 0.0f, false };

	const uint32 lightID = mLightEntityMap.at(entity);
	const Light* light = mLights[lightID].get();
	const auto pdf	   = light->pdf();
	return LightPDF{ pdf.Value * mSelector->discretePdf(lightID), pdf.IsArea };
}

} // namespace PR