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
			const float m1 = PM::pm_MaxT(MinRoughness, mRoughnessX ? mRoughnessX->eval(point) : 0);
			const float m2 = mRoughnessX == mRoughnessY ?
				m1 : PM::pm_MaxT(MinRoughness, mRoughnessY ? mRoughnessY->eval(point) : 0);

			const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Subtract(L, point.V));
			const float NdotH = PM::pm_Dot3D(point.N, H);

			if (NdotH > PM_EPSILON)
			{
				const float sf = PM::pm_ClampT(NdotL*point.NdotV, 0.01f, 1.0f);
				const float HdotX = PM::pm_Dot3D(H, point.Nx);
				const float HdotY = PM::pm_Dot3D(H, point.Ny);

				const float NdotH2 = 0.5f + 0.5f * NdotH;
				const float fx = HdotX / m1;
				const float fy = HdotY / m2;
				float r = std::exp(-(fx*fx + fy*fy) / NdotH2) * PM_INV_PI_F / (4 * m1 * m2 * std::sqrt(sf));

				if (std::isinf(r))
					r = 1;

				spec = mSpecularity->eval(point) * r;
			}
		}

		return albedo*(1-refl) + spec*refl;
	}

	float WardMaterial::pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL)
	{
		if (point.NdotV <= PM_EPSILON)
			return PM_INV_PI_F;

		const float refl = mReflectivity ? mReflectivity->eval(point) : 0.5f;
		const float m1 = PM::pm_MaxT(MinRoughness, mRoughnessX ? mRoughnessX->eval(point) : 0);
		const float m2 = mRoughnessX == mRoughnessY ?
			m1 : PM::pm_MaxT(MinRoughness, mRoughnessY ? mRoughnessY->eval(point) : 0);

		const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Subtract(L, point.V));
		const float NdotH = PM::pm_Dot3D(point.N, H);

		if (NdotH <= PM_EPSILON)
			return PM_INV_PI_F;

		const float sf = PM::pm_ClampT(NdotL*point.NdotV, 0.01f, 1.0f);
		const float HdotX = PM::pm_Dot3D(H, point.Nx);
		const float HdotY = PM::pm_Dot3D(H, point.Ny);

		const float NdotH2 = 0.5f + 0.5f * NdotH;
		const float fx = HdotX / m1;
		const float fy = HdotY / m2;

		const float r = std::exp(-(fx*fx + fy*fy) / NdotH2) * PM_INV_PI_F / (4*m1 * m2 * std::sqrt(sf));
		
		return PM::pm_MaxT(PM_INV_PI_F, (std::isinf(r) ? 1 : r)*refl);
	}

	PM::vec3 WardMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		const float m1 = PM::pm_MaxT(MinRoughness, mRoughnessX ? mRoughnessX->eval(point) : 0);
		const float m2 = mRoughnessX == mRoughnessY ?
			m1 : PM::pm_MaxT(MinRoughness, mRoughnessY ? mRoughnessY->eval(point) : 0);

		float px, py;
		if (mRoughnessX == mRoughnessY)// Isotropic
		{
			px = 2*PM_INV_PI_F*std::atan(m1 * std::sqrt(-std::log(PM::pm_MaxT(0.001f, PM::pm_GetX(rnd)))));
			py = PM::pm_GetY(rnd);
		}
		else
		{
			float s, c;
			PM::pm_SinCosT(PM_2_PI_F * PM::pm_GetY(rnd), s, c);
			const float s2 = s*s;

			px = 2*PM_INV_PI_F*std::atan(std::sqrt(-std::log(PM::pm_MaxT(0.001f, PM::pm_GetX(rnd))) * (m2*m2*s2 - m1*m1*s2 + m1*m1)));
			py = 0.5f+PM_INV_PI_F*0.5f*std::atan2(m2*s, m1*c);
		}

		float tmpPdf;
		auto H = Projection::tangent_align(point.N, point.Nx, point.Ny,
			Projection::cos_hemi(px, py, tmpPdf));
		auto dir = Reflection::reflect(PM::pm_Dot3D(H, point.V), H, point.V);
		pdf = tmpPdf*WardMaterial::pdf(point, dir, std::abs(PM::pm_Dot3D(point.N, dir)));
		return dir;
	}
}