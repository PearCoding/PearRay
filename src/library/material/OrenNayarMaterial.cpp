#include "OrenNayarMaterial.h"
#include "ray/Ray.h"

#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"

#include "shader/ConstScalarOutput.h"
#include "shader/ConstSpectralOutput.h"
#include "shader/ShaderClosure.h"

#include "math/Projection.h"

#include "BRDF.h"

#include <sstream>

namespace PR {
OrenNayarMaterial::OrenNayarMaterial(uint32 id)
	: Material(id)
	, mAlbedo(nullptr)
	, mRoughness(nullptr)
{
}

const std::shared_ptr<SpectrumShaderOutput>& OrenNayarMaterial::albedo() const
{
	return mAlbedo;
}

void OrenNayarMaterial::setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec)
{
	mAlbedo = diffSpec;
}

const std::shared_ptr<ScalarShaderOutput>& OrenNayarMaterial::roughness() const
{
	return mRoughness;
}

void OrenNayarMaterial::setRoughness(const std::shared_ptr<ScalarShaderOutput>& d)
{
	mRoughness = d;
}

constexpr float MinRoughness = 0.001f;
void OrenNayarMaterial::setup(RenderContext* context)
{
	if (!mRoughness)
		mRoughness = std::make_shared<ConstScalarShaderOutput>(0.5f);

	if (!mAlbedo)
		mAlbedo = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::black(context->spectrumDescriptor()));
}

void OrenNayarMaterial::eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	float val		= PR_1_PI;
	float roughness = mRoughness->eval(point);
	roughness *= roughness; // Square

	if (roughness > PR_EPSILON) // Oren Nayar
		val = BRDF::orennayar(roughness, point.V, point.N, L, point.NdotV, NdotL);

	mAlbedo->eval(spec, point);
	spec *= val;
}

float OrenNayarMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	return Projection::cos_hemi_pdf(NdotL);
}

MaterialSample OrenNayarMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session)
{
	MaterialSample ms;
	ms.ScatteringType = MST_DiffuseReflection;
	ms.L			  = Projection::tangent_align(point.N, point.Nx, point.Ny,
									  Projection::cos_hemi(rnd(0), rnd(1), ms.PDF_S));
	return ms;
}

std::string OrenNayarMaterial::dumpInformation() const
{
	std::stringstream stream;

	stream << std::boolalpha << Material::dumpInformation()
		   << "  <OrenNayarMaterial>:" << std::endl
		   << "    HasAlbedo:    " << (mAlbedo ? "true" : "false") << std::endl
		   << "    HasRoughness: " << (mRoughness ? "true" : "false") << std::endl;

	return stream.str();
}
} // namespace PR
