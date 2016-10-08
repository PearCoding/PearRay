#include "CookTorranceMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

#include "math/Projection.h"
#include "math/Reflection.h"
#include "math/Fresnel.h"

#include "BRDF.h"

namespace PR
{
	CookTorranceMaterial::CookTorranceMaterial() :
		Material(),
		mFresnelMode(FM_Dielectric), mDistributionMode(DM_GGX), mGeometryMode(GM_CookTorrance),
		mAlbedo(nullptr), mDiffuseRoughness(nullptr),
		mSpecularity(nullptr), mSpecRoughnessX(nullptr), mSpecRoughnessY(nullptr),
		mIOR(nullptr), mConductorAbsorption(nullptr), mReflectivity(nullptr)
	{
	}
	
	CookTorranceMaterial::FresnelMode CookTorranceMaterial::fresnelMode() const
	{
		return mFresnelMode;
	}

	void CookTorranceMaterial::setFresnelMode(FresnelMode mode)
	{
		mFresnelMode = mode;
	}

	CookTorranceMaterial::DistributionMode CookTorranceMaterial::distributionMode() const
	{
		return mDistributionMode;
	}

	void CookTorranceMaterial::setDistributionMode(DistributionMode mode)
	{
		mDistributionMode = mode;
	}

	CookTorranceMaterial::GeometryMode CookTorranceMaterial::geometryMode() const
	{
		return mGeometryMode;
	}

	void CookTorranceMaterial::setGeometryMode(GeometryMode mode)
	{
		mGeometryMode = mode;
	}

	SpectralShaderOutput* CookTorranceMaterial::albedo() const
	{
		return mAlbedo;
	}

	void CookTorranceMaterial::setAlbedo(SpectralShaderOutput* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	ScalarShaderOutput* CookTorranceMaterial::diffuseRoughness() const
	{
		return mDiffuseRoughness;
	}

	void CookTorranceMaterial::setDiffuseRoughness(ScalarShaderOutput* d)
	{
		mDiffuseRoughness = d;
	}

	SpectralShaderOutput* CookTorranceMaterial::specularity() const
	{
		return mSpecularity;
	}

	void CookTorranceMaterial::setSpecularity(SpectralShaderOutput* spec)
	{
		mSpecularity = spec;
	}

	ScalarShaderOutput* CookTorranceMaterial::specularRoughnessX() const
	{
		return mSpecRoughnessX;
	}

	void CookTorranceMaterial::setSpecularRoughnessX(ScalarShaderOutput* d)
	{
		mSpecRoughnessX = d;
	}

	ScalarShaderOutput* CookTorranceMaterial::specularRoughnessY() const
	{
		return mSpecRoughnessY;
	}

	void CookTorranceMaterial::setSpecularRoughnessY(ScalarShaderOutput* d)
	{
		mSpecRoughnessY = d;
	}

	SpectralShaderOutput* CookTorranceMaterial::ior() const
	{
		return mIOR;
	}

	void CookTorranceMaterial::setIOR(SpectralShaderOutput* data)
	{
		mIOR = data;
	}

	SpectralShaderOutput* CookTorranceMaterial::conductorAbsorption() const
	{
		return mConductorAbsorption;
	}

	void CookTorranceMaterial::setConductorAbsorption(SpectralShaderOutput* data)
	{
		mConductorAbsorption = data;
	}

	ScalarShaderOutput* CookTorranceMaterial::reflectivity() const
	{
		return mReflectivity;
	}

	void CookTorranceMaterial::setReflectivity(ScalarShaderOutput* d)
	{
		mReflectivity = d;
	}

	// Alot of potential to optimize!
	constexpr float MinRoughness = 0.001f;
	Spectrum CookTorranceMaterial::eval(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		const float refl = mReflectivity ? mReflectivity->eval(point) : 0.5f;

		Spectrum spec;
		if (refl < 1 && mAlbedo)
		{
			float val = PM_INV_PI_F;
			if (mDiffuseRoughness)
			{
				float roughness = mDiffuseRoughness->eval(point);
				roughness *= roughness;// Square

				if (roughness > PM_EPSILON)// Oren Nayar
					val = BRDF::orennayar(roughness, point.V, point.N, L, point.NdotV, NdotL);
			}// else lambert

			spec = mAlbedo->eval(point) * (val*(1-refl));
		}

		if (refl > PM_EPSILON && mSpecularity && NdotL * point.NdotV > PM_EPSILON)
		{
			const float m1 = PM::pm_MaxT(MinRoughness, mSpecRoughnessX ? mSpecRoughnessX->eval(point) : 0);

			Spectrum ind = mIOR ? mIOR->eval(point) : Spectrum(1.55f);
			Spectrum F;
			switch(mFresnelMode)
			{
				default:
				case FM_Dielectric:
					for(uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
					{
						F.setValue(i, Fresnel::dielectric(point.NdotV, 1, ind.value(i)));
					}
				break;
				case FM_Conductor:
				{
					Spectrum k = mConductorAbsorption ? mConductorAbsorption->eval(point) : Spectrum(0.0f);
					for(uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
					{
						F.setValue(i, Fresnel::conductor(point.NdotV, ind.value(i), k.value(i)));
					}
				}
				break;
			}

			const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Subtract(L, point.V));
			const float NdotH = PM::pm_Dot3D(point.N, H);

			if (NdotH > PM_EPSILON)
			{
				float G;// Includes 1/NdotL*NdotV
				switch(mGeometryMode)
				{
					case GM_Implicit:
						G = BRDF::g_implicit(point.NdotV, NdotL);
						break;
					case GM_Neumann:
						G = BRDF::g_neumann(point.NdotV, NdotL);
						break;
					default:
					case GM_CookTorrance:
						G = BRDF::g_cooktorrance(point.NdotV, NdotL, NdotH, -PM::pm_Dot3D(point.V, H));
						break;
					case GM_Kelemen:
						G = BRDF::g_kelemen(point.NdotV, NdotL, PM::pm_Dot3D(point.V, H));// No need for -, will be quadrated anyway
						break;
				}

				float D;
				switch(mDistributionMode)
				{
					case DM_Blinn:
						D = BRDF::ndf_blinn(NdotH, m1);
						break;
					case DM_Beckmann:
						D = BRDF::ndf_beckmann(NdotH, m1);
						break;
					default:
					case DM_GGX:
						if(mSpecRoughnessX == mSpecRoughnessY)
							D = BRDF::ndf_ggx_iso(NdotH, m1);
						else
						{
							const float m2 = PM::pm_MaxT(MinRoughness, mSpecRoughnessY ? mSpecRoughnessY->eval(point) : 0);

							const float XdotH = PM::pm_Dot3D(point.Nx, H);
							const float YdotH = PM::pm_Dot3D(point.Ny, H);

							D = BRDF::ndf_ggx_aniso(NdotH, XdotH, YdotH, m1, m2);
						}
						break;
				}

				spec += mSpecularity->eval(point) * F * (0.25f * D * G * refl);
			}
		}

		return spec;
	}

	float CookTorranceMaterial::pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		const float refl = mReflectivity ? mReflectivity->eval(point) : 0.5f;
		const float m1 = PM::pm_MaxT(MinRoughness, mSpecRoughnessX ? mSpecRoughnessX->eval(point) : 0);

		const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Subtract(L, point.V));
		const float NdotH = PM::pm_Dot3D(point.N, H);
		const float prod = NdotL * point.NdotV;

		if (NdotH <= PM_EPSILON || prod <= PM_EPSILON)
			return 0;

		float D;
		switch(mDistributionMode)
		{
			case DM_Blinn:
				D = BRDF::ndf_blinn(NdotH, m1);
				break;
			case DM_Beckmann:
				D = BRDF::ndf_beckmann(NdotH, m1);
				break;
			case DM_GGX:
			default:
				if(mSpecRoughnessX == mSpecRoughnessY)
					D = BRDF::ndf_ggx_iso(NdotH, m1);
				else
				{
					const float m2 = PM::pm_MaxT(MinRoughness, mSpecRoughnessY ? mSpecRoughnessY->eval(point) : 0);

					const float XdotH = PM::pm_Dot3D(point.Nx, H);
					const float YdotH = PM::pm_Dot3D(point.Ny, H);

					D = BRDF::ndf_ggx_aniso(NdotH, XdotH, YdotH, m1, m2);
				}
				break;
		}

		return PM::pm_ClampT(Projection::cos_hemi_pdf(NdotL) * (1-refl) + (0.25f*D/prod)*refl, 0.0f, 1.0f);
	}

	PM::vec3 CookTorranceMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		const float refl = mReflectivity ? mReflectivity->eval(point) : 0.5f;

		if(PM::pm_GetZ(rnd) < refl)
			return diffuse_path(point, rnd, pdf);
		else
			return specular_path(point, rnd, pdf);
	}

	PM::vec3 CookTorranceMaterial::samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, float& weight, uint32 path)
	{
		weight = mReflectivity ? mReflectivity->eval(point) : 0.5f;

		if(path == 0)
			return diffuse_path(point, rnd, pdf);
		else
			return specular_path(point, rnd, pdf);
	}

	uint32 CookTorranceMaterial::samplePathCount() const
	{
		return 2;
	}

	PM::vec3 CookTorranceMaterial::diffuse_path(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		return Projection::tangent_align(point.N, point.Nx, point.Ny,
			Projection::cos_hemi(PM::pm_GetX(rnd), PM::pm_GetY(rnd), pdf));
	}

	PM::vec3 CookTorranceMaterial::specular_path(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		float u = PM::pm_GetX(rnd);
		float v = PM::pm_GetY(rnd);

		float m1 = mSpecRoughnessX ? mSpecRoughnessX->eval(point) : 0;
		float cosTheta, sinTheta;// V samples
		float cosPhi, sinPhi;// U samples

		switch(mDistributionMode)
		{
			case DM_Blinn:
			{
				if(m1 <= PM_EPSILON)
					cosTheta = 1;
				else
					cosTheta = std::pow(1 - v, 1/(2*m1*m1));
					
				PM::pm_SinCosT(PM_2_PI_F * u, sinPhi, cosPhi);
			}
				break;
			default:
			case DM_Beckmann:
			{
				float t = 1/(1-m1*m1*std::log(1-v));
				cosTheta = std::sqrt(t);
				PM::pm_SinCosT(PM_2_PI_F * u, sinPhi, cosPhi);
			}
				break;
			case DM_GGX:
			{
				m1 = PM::pm_MaxT(MinRoughness, m1);
				float r2;
				float alpha2;
				if(mSpecRoughnessX == mSpecRoughnessY)
				{
					PM::pm_SinCosT(PM_2_PI_F * u, sinPhi, cosPhi);
					alpha2 = m1*m1;
					r2 = alpha2;
				}
				else
				{
					const float m2 = PM::pm_MaxT(MinRoughness, mSpecRoughnessY ? mSpecRoughnessY->eval(point) : 0);

					float phi = std::atan(m2/m1*std::tan(PM_PI_F + PM_2_PI_F * u)) +
						PM_PI_F * std::floor(2*u + 0.5f);
					PM::pm_SinCosT(phi, sinPhi, cosPhi);

					float f1 = cosPhi/m1;
					float f2 = sinPhi/m2;
					alpha2 = 1/(f1*f1 + f2*f2);
					r2 = m1*m2;
				}

				float t2 = alpha2 * v / (1-v);
				cosTheta = PM::pm_MaxT(0.001f, 1.0f / std::sqrt(1 + t2));

				float s = 1+t2/alpha2;
				pdf = PM_INV_PI_F / (r2*cosTheta*cosTheta*cosTheta*s*s);
			}
				break;
		}
		
		sinTheta = std::sqrt(1-cosTheta*cosTheta);
		auto H = Projection::tangent_align(point.N, point.Nx, point.Ny,
			PM::pm_Set(sinTheta*cosPhi, sinTheta*sinPhi, cosTheta));
		auto dir = PM::pm_Normalize3D(Reflection::reflect(std::abs(PM::pm_Dot3D(H, point.V)), H, point.V));

		if(mDistributionMode != DM_GGX)//TODO: Calculate it!
			pdf = CookTorranceMaterial::pdf(point, dir, std::abs(PM::pm_Dot3D(point.N, dir)));
		else
			pdf /= 4*std::abs(PM::pm_Dot3D(point.N, dir)) * point.NdotV;
		
		pdf = PM::pm_ClampT<float>(pdf, 0, 1);
		return dir;
	}
}