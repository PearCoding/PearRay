// IWYU pragma: private, include "light/LightSampler.h"

namespace PR {
inline const Light* LightSampler::sample(float rnd, float& pdf) const
{
	PR_ASSERT(mSelector, "Expected initialized light sampler");
	const size_t id = mSelector->sampleDiscrete(rnd, pdf);
	return id < mLights.size() ? mLights[id].get() : nullptr;
}

inline std::pair<const Light*, float> LightSampler::sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const
{
	PR_ASSERT(mSelector, "Expected initialized light sampler");
	float pdf, rem;
	const size_t id = mSelector->sampleDiscrete(in.RND[0], pdf, &rem);
	if (id < mLights.size()) {
		const Light* l = mLights[id].get();

		LightSampleInput in2 = in;
		in2.RND[0]			 = rem;
		l->sample(in2, out, session);
		return { l, pdf };
	} else {
		return { nullptr, 0.0f };
	}
}

float LightSampler::pdfSelection(const IEntity* entity) const
{
	if (!entity)
		return mInfLightSelectionProbability;

	if (!entity->hasEmission() || mLightEntityMap.count(entity) == 0)
		return 0.0f;

	const uint32 lightID = mLightEntityMap.at(entity);
	return mSelector->discretePdf(lightID);
}

LightPDF LightSampler::pdfPosition(const IEntity* entity, const EntitySamplingInfo* info) const
{
	if (!entity)
		return LightPDF{ PR_INV_2_PI, false }; // TODO

	if (!entity->hasEmission() || mLightEntityMap.count(entity) == 0)
		return LightPDF{ 0.0f, false };

	const uint32 lightID = mLightEntityMap.at(entity);
	const Light* light	 = mLights[lightID].get();
	return light->pdfPosition(info);
}

inline float LightSampler::pdfDirection(const Vector3f& dir, const IEntity* entity, const EntitySamplingInfo* info) const
{
	if (!entity) {
		float s = 0;
		for (const auto& light : mInfLights)
			s += light->pdfDirection(dir, info);
		return s;
	}

	if (!entity->hasEmission() || mLightEntityMap.count(entity) == 0)
		return 0.0f;

	const uint32 lightID = mLightEntityMap.at(entity);
	const Light* light	 = mLights[lightID].get();
	return light->pdfDirection(dir, info);
}

inline const Light* LightSampler::light(const IEntity* entity) const
{
	if (!entity || mLightEntityMap.count(entity) == 0)
		return nullptr;

	return mLights[mLightEntityMap.at(entity)].get();
}

} // namespace PR