#include "DiffuseMaterial.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "shader/ShaderClosure.h"
#include "shader/ConstSpectralOutput.h"

#include "math/Projection.h"

#include <sstream>

namespace PR {
DiffuseMaterial::DiffuseMaterial(uint32 id)
	: Material(id)
	, mAlbedo(nullptr)
{
}

std::shared_ptr<SpectrumShaderOutput> DiffuseMaterial::albedo() const
{
	return mAlbedo;
}

void DiffuseMaterial::setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec)
{
	mAlbedo = diffSpec;
}

void DiffuseMaterial::eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const
{
	mAlbedo->eval(spec, point);
	spec *= PR_1_PI;
}

float DiffuseMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session) const
{
	return Projection::cos_hemi_pdf(NdotL);
}

MaterialSample DiffuseMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session) const
{
	MaterialSample ms;
	ms.ScatteringType = MST_DiffuseReflection;
	ms.L			  = Projection::tangent_align(point.N, point.Nx, point.Ny,
									  Projection::cos_hemi(rnd(0), rnd(1), ms.PDF_S));
	return ms;
}

void DiffuseMaterial::onFreeze(RenderContext* context)
{
	if (!mAlbedo)
		mAlbedo = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::white(context->spectrumDescriptor()));
}

std::string DiffuseMaterial::dumpInformation() const
{
	std::stringstream stream;

	stream << std::boolalpha << Material::dumpInformation()
		   << "  <DiffuseMaterial>:" << std::endl
		   << "    HasAlbedo: " << (mAlbedo ? "true" : "false") << std::endl;

	return stream.str();
}
} // namespace PR
