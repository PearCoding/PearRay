#include "WardMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

#include "math/Projection.h"
#include "math/Reflection.h"
#include "math/Fresnel.h"

namespace PR
{
	WardMaterial::WardMaterial() :
		Material(), mAlbedo(nullptr), mSpecularity(nullptr),
		mRoughnessX(nullptr), mRoughnessY(nullptr), mReflectivity(nullptr)
	{
	}

	SpectralShaderOutput* WardMaterial::albedo() const
	{
		return mAlbedo;
	}

	void WardMaterial::setAlbedo(SpectralShaderOutput* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	SpectralShaderOutput* WardMaterial::specularity() const
	{
		return mSpecularity;
	}

	void WardMaterial::setSpecularity(SpectralShaderOutput* spec)
	{
		mSpecularity = spec;
	}

	ScalarShaderOutput* WardMaterial::roughnessX() const
	{
		return mRoughnessX;
	}

	void WardMaterial::setRoughnessX(ScalarShaderOutput* d)
	{
		mRoughnessX = d;
	}

	ScalarShaderOutput* WardMaterial::roughnessY() const
	{
		return mRoughnessY;
	}

	void WardMaterial::setRoughnessY(ScalarShaderOutput* d)
	{
		mRoughnessY = d;
	}

	ScalarShaderOutput* WardMaterial::reflectivity() const
	{
		return mReflectivity;
	}

	void WardMaterial::setReflectivity(ScalarShaderOutput* d)
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
			const float prod = NdotL*point.NdotV;

			if (NdotH > PM_EPSILON && prod > PM_EPSILON)
			{
				const float HdotX = std::abs(PM::pm_Dot3D(H, point.Nx));
				const float HdotY = std::abs(PM::pm_Dot3D(H, point.Ny));

				const float NdotH2 = 0.5f + 0.5f * NdotH;
				const float fx = HdotX / m1;
				const float fy = HdotY / m2;
				float r = std::exp(-(fx*fx + fy*fy) / NdotH2) * PM_INV_PI_F / (4 * m1 * m2 * std::sqrt(prod));

				if(r > PM_EPSILON)
					spec = mSpecularity->eval(point) * r;
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
		const float prod = NdotL*point.NdotV;

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

	PM::vec3 WardMaterial::samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, float& weight, uint32 path)
	{
		weight = mReflectivity ? mReflectivity->eval(point) : 0.5f;

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

			const float f2 = - std::log(PM::pm_Max(0.001f, v)) / (cosPhi*cosPhi/pm1 + sinPhi*sinPhi/pm2);
			cosTheta = 1/(1+f2);
			sinTheta = PM::pm_SafeSqrt(f2)*cosTheta;
		}

		auto H = Projection::tangent_align(point.N, point.Nx, point.Ny,
			PM::pm_Set(sinTheta*cosPhi, sinTheta*sinPhi, cosTheta));
		auto dir = Reflection::reflect(std::abs(PM::pm_Dot3D(H, point.V)), H, point.V);

		if (mRoughnessX == mRoughnessY)
			pdf /= std::abs(PM::pm_Dot3D(point.V, dir));
		else
			pdf = WardMaterial::pdf(point, dir, std::abs(PM::pm_Dot3D(point.N, dir)));
		
		return dir;
	}
}