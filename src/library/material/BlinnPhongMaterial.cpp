#include "BlinnPhongMaterial.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"
#include "shader/ConstScalarOutput.h"
#include "shader/ConstSpectralOutput.h"

#include "math/Fresnel.h"
#include "math/Projection.h"
#include "math/Reflection.h"
#include "shader/ShaderClosure.h"

#include <sstream>

namespace PR {
BlinnPhongMaterial::BlinnPhongMaterial(uint32 id)
	: Material(id)
	, mAlbedo(nullptr)
	, mShininess(nullptr)
	, mIndex(nullptr)
{
}

std::shared_ptr<SpectrumShaderOutput> BlinnPhongMaterial::albedo() const
{
	return mAlbedo;
}

void BlinnPhongMaterial::setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec)
{
	mAlbedo = diffSpec;
}

std::shared_ptr<ScalarShaderOutput> BlinnPhongMaterial::shininess() const
{
	return mShininess;
}

void BlinnPhongMaterial::setShininess(const std::shared_ptr<ScalarShaderOutput>& d)
{
	mShininess = d;
}

std::shared_ptr<SpectrumShaderOutput> BlinnPhongMaterial::fresnelIndex() const
{
	return mIndex;
}

void BlinnPhongMaterial::setFresnelIndex(const std::shared_ptr<SpectrumShaderOutput>& data)
{
	mIndex = data;
}

struct BPM_ThreadData {
	Spectrum Index;

	explicit BPM_ThreadData(RenderContext* context)
		: Index(context->spectrumDescriptor())
	{
	}
};

void BlinnPhongMaterial::onFreeze(RenderContext* context)
{
	mThreadData.clear();
	for (size_t i = 0; i < context->threads(); ++i) {
		mThreadData.push_back(std::make_shared<BPM_ThreadData>(context));
	}

	if (!mAlbedo)
		mAlbedo = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::black(context->spectrumDescriptor()));

	if (!mIndex)
		mIndex = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::gray(context->spectrumDescriptor(), 1.55f));

	if (!mShininess)
		mShininess = std::make_shared<ConstScalarShaderOutput>(0.0f);
}

// TODO: Should be normalized better.
void BlinnPhongMaterial::eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const
{
	const std::shared_ptr<BPM_ThreadData>& data = mThreadData[session.thread()];
	mAlbedo->eval(spec, point);
	spec *= PR_1_PI;

	const Eigen::Vector3f H = Reflection::halfway(L, point.V);
	const float NdotH		= std::abs(point.N.dot(H));
	const float VdotH		= std::abs(point.V.dot(H));

	float n = mShininess->eval(point);

	mIndex->eval(data->Index, point);

	const float factor = std::pow(NdotH, n) * PR_1_PI * (n + 4) / 8;

	for (uint32 i = 0; i < spec.samples(); ++i) {
		const float n2 = data->Index.value(i);
		const float f  = Fresnel::dielectric(VdotH,
											 !(point.Flags & SCF_Inside) ? 1 : n2,
											 !(point.Flags & SCF_Inside) ? n2 : 1);

		spec.setValue(i, spec.value(i) + f * factor);
	}
}

float BlinnPhongMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const
{
	const Eigen::Vector3f H = Reflection::halfway(L, point.V);
	const float NdotH		= std::abs(point.N.dot(H));
	float n					= 0;
	mShininess->eval(n, point);
	return PR_1_PI + std::pow(NdotH, n);
}

MaterialSample BlinnPhongMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) const
{
	MaterialSample ms;
	ms.L = Projection::tangent_align(point.N,
									 Projection::cos_hemi(rnd(0), rnd(1), ms.PDF_S));
	ms.PDF_S += BlinnPhongMaterial::pdf(point, ms.L, 0, session);
	ms.ScatteringType = MST_DiffuseReflection;
	return ms;
}

std::string BlinnPhongMaterial::dumpInformation() const
{
	std::stringstream stream;

	stream << std::boolalpha << Material::dumpInformation()
		   << "  <BlinnPhongMaterial>:" << std::endl
		   << "    HasAlbedo:    " << (mAlbedo ? "true" : "false") << std::endl
		   << "    HasShininess: " << (mShininess ? "true" : "false") << std::endl
		   << "    HasIOR:       " << (mIndex ? "true" : "false") << std::endl;

	return stream.str();
}
} // namespace PR
