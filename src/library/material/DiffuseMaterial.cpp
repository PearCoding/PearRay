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

	SpectralShaderOutput* DiffuseMaterial::albedo() const
	{
		return mAlbedo;
	}

	void DiffuseMaterial::setAlbedo(SpectralShaderOutput* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	Spectrum DiffuseMaterial::eval(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		if (mAlbedo)
			return mAlbedo->eval(point) * PM_INV_PI_F;
		else
			return Spectrum();
	}

	float DiffuseMaterial::pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		return Projection::cos_hemi_pdf(NdotL);
			//1;
	}

	PM::vec3 DiffuseMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		auto dir = Projection::tangent_align(point.N, point.Nx, point.Ny,
			Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), pdf));
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
