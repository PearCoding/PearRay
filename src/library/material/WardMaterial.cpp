#include "WardMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

#include "math/Projection.h"
#include "math/Reflection.h"
#include "math/Fresnel.h"

#include <sstream>

namespace PR
{
	WardMaterial::WardMaterial(uint32 id) :
		Material(id), mAlbedo(nullptr), mSpecularity(nullptr),
		mRoughnessX(nullptr), mRoughnessY(nullptr), mReflectivity(nullptr)
	{
	}

	const std::shared_ptr<SpectralShaderOutput>& WardMaterial::albedo() const
	{
		return mAlbedo;
	}

	void WardMaterial::setAlbedo(const std::shared_ptr<SpectralShaderOutput>& diffSpec)
	{
		mAlbedo = diffSpec;
	}

	const std::shared_ptr<SpectralShaderOutput>& WardMaterial::specularity() const
	{
		return mSpecularity;
	}

	void WardMaterial::setSpecularity(const std::shared_ptr<SpectralShaderOutput>& spec)
	{
		mSpecularity = spec;
	}

	const std::shared_ptr<ScalarShaderOutput>& WardMaterial::roughnessX() const
	{
		return mRoughnessX;
	}

	void WardMaterial::setRoughnessX(const std::shared_ptr<ScalarShaderOutput>& d)
	{
		mRoughnessX = d;
	}

	const std::shared_ptr<ScalarShaderOutput>& WardMaterial::roughnessY() const
	{
		return mRoughnessY;
	}

	void WardMaterial::setRoughnessY(const std::shared_ptr<ScalarShaderOutput>& d)
	{
		mRoughnessY = d;
	}

	const std::shared_ptr<ScalarShaderOutput>& WardMaterial::reflectivity() const
	{
		return mReflectivity;
	}

	void WardMaterial::setReflectivity(const std::shared_ptr<ScalarShaderOutput>& d)
	{
		mReflectivity = d;
	}

	constexpr float MinRoughness = 0.001f;
	Spectrum WardMaterial::eval(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		const float refl = mReflectivity ? mReflectivity->eval(point) : 0.5f;

		Spectrum albedo;
		if (mAlbedo)
		{
			albedo = mAlbedo->eval(point) * PM_INV_PI_F;
		}

		Spectrum spec;
		if (mSpecularity && mRoughnessX && mRoughnessY)
		{
			const float m1 = PM::pm_Max(MinRoughness, mRoughnessX ? mRoughnessX->eval(point) : 0);
			const float m2 = mRoughnessX == mRoughnessY ?
				m1 : PM::pm_Max(MinRoughness, mRoughnessY ? mRoughnessY->eval(point) : 0);

			const PM::vec3 H = Reflection::halfway(point.V, L);
			const float NdotH = PM::pm_Dot3D(point.N, H);
			const float prod = -NdotL*point.NdotV;

			if (NdotH > PM_EPSILON && prod > PM_EPSILON)
			{
				const float HdotX = std::abs(PM::pm_Dot3D(H, point.Nx));
				const float HdotY = std::abs(PM::pm_Dot3D(H, point.Ny));

				const float NdotH2 = 0.5f + 0.5f * NdotH;
				const float fx = HdotX / m1;
				const float fy = HdotY / m2;
				float r = std::exp(-(fx*fx + fy*fy) / NdotH2) * PM_INV_PI_F / (4 * m1 * m2 * std::sqrt(prod));

				if(r > PM_EPSILON)
					spec = mSpecularity->eval(point) * PM::pm_Min(r, 1.0f);
			}
		}

		return albedo*(1-refl) + spec*refl;
	}

	float WardMaterial::pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		const float refl = mReflectivity ? mReflectivity->eval(point) : 0.5f;
		const float m1 = PM::pm_Max(MinRoughness, mRoughnessX ? mRoughnessX->eval(point) : 0);
		const float m2 = mRoughnessX == mRoughnessY ?
			m1 : PM::pm_Max(MinRoughness, mRoughnessY ? mRoughnessY->eval(point) : 0);

		const PM::vec3 H = Reflection::halfway(point.V, L);
		const float NdotH = PM::pm_Dot3D(point.N, H);
		const float prod = -NdotL*point.NdotV;

		if (NdotH <= PM_EPSILON || prod <= PM_EPSILON)
			return PM_INV_PI_F;

		const float HdotX = std::abs(PM::pm_Dot3D(H, point.Nx));
		const float HdotY = std::abs(PM::pm_Dot3D(H, point.Ny));

		const float NdotH2 = 0.5f + 0.5f * NdotH;
		const float fx = HdotX / m1;
		const float fy = HdotY / m2;

		const float r = std::exp(-(fx*fx + fy*fy) / NdotH2) * PM_INV_PI_F / (4 * m1 * m2 * std::sqrt(prod));

		return PM::pm_Clamp(Projection::cos_hemi_pdf(NdotL) * (1-refl) + r*refl, 0.0f, 1.0f);
	}

	PM::vec3 WardMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		const float refl = mReflectivity ? mReflectivity->eval(point) : 0.5f;

		if(PM::pm_GetZ(rnd) < refl)
			return diffuse_path(point, rnd, pdf);
		else
			return specular_path(point, rnd, pdf);
	}

	PM::vec3 WardMaterial::samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, float& path_weight, uint32 path)
	{
		path_weight = mReflectivity ? mReflectivity->eval(point) : 0.5f;

		if(path == 0)
			return diffuse_path(point, rnd, pdf);
		else
			return specular_path(point, rnd, pdf);
	}

	uint32 WardMaterial::samplePathCount() const
	{
		return 2;
	}

	PM::vec3 WardMaterial::diffuse_path(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		return Projection::tangent_align(point.N, point.Nx, point.Ny,
			Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), pdf));
	}

	PM::vec3 WardMaterial::specular_path(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		float u = PM::pm_GetX(rnd);
		float v = PM::pm_GetY(rnd);

		const float m1 = PM::pm_Max(MinRoughness, mRoughnessX ? mRoughnessX->eval(point) : 0);

		float cosTheta, sinTheta;// V samples
		float cosPhi, sinPhi;// U samples
		if (mRoughnessX == mRoughnessY)// Isotropic
		{
			PM::pm_SinCos(PM_2_PI_F * u, sinPhi, cosPhi);

			const float f = - std::log(PM::pm_Max(0.001f, v))*m1*m1;
			cosTheta = 1/(1+f);
			sinTheta = PM::pm_SafeSqrt(f)*cosTheta;

			const float t = PM_4_PI_F * m1 * m1 * cosTheta * cosTheta * cosTheta * v;
			if(t <= PM_EPSILON)
				pdf = 1;
			else
				pdf = 1 / t;
		}
		else
		{
			const float m2 = PM::pm_Max(MinRoughness, mRoughnessY ? mRoughnessY->eval(point) : 0);

			const float pm1 = PM::pm_Max(MinRoughness, m1*m1);
			const float pm2 = PM::pm_Max(MinRoughness, m2*m2);

			const float f1 = (m2/m1)*std::tan(PM_2_PI_F*u);
			cosPhi = 1/PM::pm_SafeSqrt(1+f1*f1);
			sinPhi = f1*cosPhi;

			const float cosPhi2 = cosPhi * cosPhi;
			const float tz = (cosPhi2/pm1 + sinPhi*sinPhi/pm2);
			const float f2 = - std::log(PM::pm_Max(0.001f, v)) / tz;
			cosTheta = 1/(1+f2);
			sinTheta = PM::pm_SafeSqrt(f2)*cosTheta;

			const float cosTheta2 = cosTheta * cosTheta;
			const float tu = pm1*sinPhi*sinPhi + pm2*cosPhi2;
			const float tb = 4 * PM_PI_F * m1 * m2 * (pm1*(1-cosPhi2)/cosPhi + pm2*cosPhi)*cosTheta2;
			pdf = tu/tb * std::exp(-tz * (1 - cosTheta2) / (cosTheta2));
		}

		auto H = Projection::tangent_align(point.N, point.Nx, point.Ny,
			PM::pm_Set(sinTheta*cosPhi, sinTheta*sinPhi, cosTheta));
		auto dir = Reflection::reflect(std::abs(PM::pm_Dot3D(H, point.V)), H, point.V);

		pdf = PM::pm_Clamp(pdf/std::abs(PM::pm_Dot3D(point.V, dir)), 0.0f, 1.0f);

		return dir;
	}

	std::string WardMaterial::dumpInformation() const
	{
		std::stringstream stream;

		stream << std::boolalpha << Material::dumpInformation()
		    << "  <WardMaterial>:" << std::endl
			<< "    HasAlbedo:       " << (mAlbedo ? "true" :  "false") << std::endl
			<< "    HasSpecularity:  " << (mSpecularity ? "true" :  "false") << std::endl
			<< "    HasRoughnessX:   " << (mRoughnessX ? "true" :  "false") << std::endl
			<< "    HasRoughnessY:   " << (mRoughnessY ? "true" :  "false") << std::endl
			<< "    HasReflectivity: " << (mReflectivity ? "true" :  "false") << std::endl;

		return stream.str();
	}
}
