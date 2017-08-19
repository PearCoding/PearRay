#include "BlinnPhongMaterial.h"
#include "ray/Ray.h"

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

const std::shared_ptr<SpectralShaderOutput>& BlinnPhongMaterial::albedo() const
{
	return mAlbedo;
}

void BlinnPhongMaterial::setAlbedo(const std::shared_ptr<SpectralShaderOutput>& diffSpec)
{
	mAlbedo = diffSpec;
}

const std::shared_ptr<ScalarShaderOutput>& BlinnPhongMaterial::shininess() const
{
	return mShininess;
}

void BlinnPhongMaterial::setShininess(const std::shared_ptr<ScalarShaderOutput>& d)
{
	mShininess = d;
}

const std::shared_ptr<SpectralShaderOutput>& BlinnPhongMaterial::fresnelIndex() const
{
	return mIndex;
}

void BlinnPhongMaterial::setFresnelIndex(const std::shared_ptr<SpectralShaderOutput>& data)
{
	mIndex = data;
}

// TODO: Should be normalized better.
Spectrum BlinnPhongMaterial::eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
{
	Spectrum albedo;
	if (mAlbedo)
		albedo = mAlbedo->eval(point) * PR_1_PI;

	Spectrum spec;
	if (mIndex && mShininess) {
		const Eigen::Vector3f H = Reflection::halfway(L, point.V);
		const float NdotH		= std::abs(point.N.dot(H));
		const float VdotH		= std::abs(point.V.dot(H));
		const float n			= mShininess->eval(point);
		Spectrum index			= mIndex->eval(point);

		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i) {
			const float n2 = index.value(i);
			const float f  = Fresnel::dielectric(VdotH,
												!(point.Flags & SCF_Inside) ? 1 : n2,
												!(point.Flags & SCF_Inside) ? n2 : 1);

			spec.setValue(i, f);
		}

		spec *= std::pow(NdotH, n) * PR_1_PI * (n + 4) / 8;
	}

	return albedo + spec;
}

float BlinnPhongMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
{
	if (mIndex) {
		const Eigen::Vector3f H = Reflection::halfway(L, point.V);
		const float NdotH		= std::abs(point.N.dot(H));
		const float n			= mShininess->eval(point);
		return PR_1_PI + std::pow(NdotH, n);
	} else {
		return PR_1_PI;
	}
}

MaterialSample BlinnPhongMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd)
{
	MaterialSample ms;
	ms.L = Projection::tangent_align(point.N,
										 Projection::cos_hemi(rnd(0), rnd(1), ms.PDF_S));
	ms.PDF_S += BlinnPhongMaterial::pdf(point, ms.L, 0);
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
}
