#include "DiffuseMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

#include "math/Projection.h"

#include <sstream>

namespace PR
{
	DiffuseMaterial::DiffuseMaterial(uint32 id) :
		Material(id), mAlbedo(nullptr)
	{
	}

	const std::shared_ptr<SpectralShaderOutput>& DiffuseMaterial::albedo() const
	{
		return mAlbedo;
	}

	void DiffuseMaterial::setAlbedo(const std::shared_ptr<SpectralShaderOutput>& diffSpec)
	{
		mAlbedo = diffSpec;
	}

	Spectrum DiffuseMaterial::eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
	{
		if (mAlbedo)
			return mAlbedo->eval(point) * PR_1_PI;
		else
			return Spectrum();
	}

	float DiffuseMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
	{
		return Projection::cos_hemi_pdf(NdotL);
	}

	Eigen::Vector3f DiffuseMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf)
	{
		auto dir = Projection::tangent_align(point.N, point.Nx, point.Ny,
			Projection::cos_hemi(rnd(0), rnd(1), pdf));
		return dir;
	}

	std::string DiffuseMaterial::dumpInformation() const
	{
		std::stringstream stream;

		stream << std::boolalpha << Material::dumpInformation()
		    << "  <DiffuseMaterial>:" << std::endl
			<< "    HasAlbedo: " << (mAlbedo ? "true" : "false") << std::endl;

		return stream.str();
	}
}
