// IWYU pragma: private, include "light/LightSampler.h"

namespace PR {
inline const Light* LightSampler::sample(float rnd, float& pdf) const
{
	if (mSelector) {
		const size_t id = mSelector->sampleDiscrete(rnd, pdf);
		return id < mLights.size() ? mLights[id].get() : nullptr;
	} else { // No lights available
		return nullptr;
	}
}

inline std::pair<const Light*, float> LightSampler::sample(const LightSampleInput& in, LightSampleOutput& out, const RenderTileSession& session) const
{
	if (mSelector) {
		float pdf, rem;
		const size_t id = mSelector->sampleDiscrete(in.RND.getFloat(), pdf, &rem);
		if (id < mLights.size()) {
			const Light* l = mLights[id].get();
			l->sample(in, out, session);
			return { l, pdf };
		}
	}

	// No lights available
	return { nullptr, 0.0f };
}

inline float LightSampler::pdfLightSelection(const Light* light) const
{
	PR_ASSERT(mSelector, "Expected initialized light sampler"); // If you get here!
	PR_ASSERT(light, "Invalid light given");
	PR_ASSERT(light->lightID() < mLights.size(), "Light not member of sampler");
	return mSelector->discretePdf(light->lightID());
}

float LightSampler::pdfEntitySelection(const IEntity* entity) const
{
	if (!entity)
		return mInfLightSelectionProbability;

	if (!entity->hasEmission() || mLightEntityMap.count(entity) == 0)
		return 0.0f;

	const uint32 lightID = mLightEntityMap.at(entity);
	return mSelector->discretePdf(lightID);
}

LightPDF LightSampler::pdfPosition(const IEntity* entity, const Vector3f& posOnLight, const EntitySamplingInfo* info) const
{
	if (!entity)
		return LightPDF{ PR_INV_2_PI, false }; // TODO

	if (!entity->hasEmission() || mLightEntityMap.count(entity) == 0)
		return LightPDF{ 0.0f, false };

	const uint32 lightID = mLightEntityMap.at(entity);
	const Light* light	 = mLights[lightID].get();
	return light->pdfPosition(posOnLight, info);
}

inline float LightSampler::pdfDirection(const Vector3f& dir, const IEntity* entity, float cosLight) const
{
	if (!entity) {
		float s = 0;
		for (const auto& light : mInfLights)
			s += light->pdfDirection(dir, cosLight);
		return s;
	}

	if (!entity->hasEmission() || mLightEntityMap.count(entity) == 0)
		return 0.0f;

	const uint32 lightID = mLightEntityMap.at(entity);
	const Light* light	 = mLights[lightID].get();
	return light->pdfDirection(dir, cosLight);
}

inline const Light* LightSampler::light(const IEntity* entity) const
{
	if (!entity || mLightEntityMap.count(entity) == 0)
		return nullptr;

	return mLights[mLightEntityMap.at(entity)].get();
}

} // namespace PR