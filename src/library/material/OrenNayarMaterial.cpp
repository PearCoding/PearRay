#include "OrenNayarMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

#include "math/Projection.h"

#include "BRDF.h"

#include <sstream>

namespace PR
{
	OrenNayarMaterial::OrenNayarMaterial(uint32 id) :
		Material(id), mAlbedo(nullptr), mRoughness(nullptr)
	{
	}

	const std::shared_ptr<SpectralShaderOutput>& OrenNayarMaterial::albedo() const
	{
		return mAlbedo;
	}

	void OrenNayarMaterial::setAlbedo(const std::shared_ptr<SpectralShaderOutput>& diffSpec)
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

	Spectrum OrenNayarMaterial::eval(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		if (mAlbedo)
		{
			float val = PM_INV_PI_F;
			if (mRoughness)
			{
				float roughness = mRoughness->eval(point);
				roughness *= roughness;// Square

				if (roughness > PM_EPSILON)// Oren Nayar
					val = BRDF::orennayar(roughness, point.V, point.N, L, point.NdotV, NdotL);
			}// else lambert

			return mAlbedo->eval(point) * val;
		}
		else
			return Spectrum();
	}

	float OrenNayarMaterial::pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		return Projection::cos_hemi_pdf(NdotL);
	}

	PM::vec3 OrenNayarMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		auto dir = Projection::tangent_align(point.N, point.Nx, point.Ny,
			Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), pdf));
		return dir;
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
}
