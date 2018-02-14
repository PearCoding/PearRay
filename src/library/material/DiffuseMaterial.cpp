#include "DiffuseMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

#include "math/Projection.h"

#include <sstream>

namespace PR {
DiffuseMaterial::DiffuseMaterial(uint32 id)
	: Material(id)
	, mAlbedo(nullptr)
{
}

const std::shared_ptr<SpectrumShaderOutput>& DiffuseMaterial::albedo() const
{
	return mAlbedo;
}

void DiffuseMaterial::setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec)
{
	mAlbedo = diffSpec;
}

void DiffuseMaterial::eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	if (mAlbedo) {
		mAlbedo->eval(spec, point);
		spec *= PR_1_PI;
	}
}

float DiffuseMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	return Projection::cos_hemi_pdf(NdotL);
}

MaterialSample DiffuseMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session)
{
	MaterialSample ms;
	ms.ScatteringType = MST_DiffuseReflection;
	ms.L			  = Projection::tangent_align(point.N, point.Nx, point.Ny,
									  Projection::cos_hemi(rnd(0), rnd(1), ms.PDF_S));
	return ms;
}

void DiffuseMaterial::setup(RenderContext* context)
{
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
