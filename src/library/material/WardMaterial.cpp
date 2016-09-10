#include "WardMaterial.h"
#include "ray/Ray.h"
#include "shader/SamplePoint.h"

#include "math/Projection.h"
#include "math/Reflection.h"
#include "math/Fresnel.h"

namespace PR
{
	WardMaterial::WardMaterial() :
		Material(), mAlbedo(nullptr), mSpecularity(nullptr), mRoughnessX(nullptr), mRoughnessY(nullptr)
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

	constexpr float MinRoughness = 0.001f;
	Spectrum WardMaterial::apply(const SamplePoint& point, const PM::vec3& L)
	{
		Spectrum albedo;
		if (mAlbedo)
		{
			albedo = mAlbedo->eval(point) * PM_INV_PI_F;
		}

		Spectrum spec;
		if (mSpecularity && mRoughnessX && mRoughnessY)
		{
			const float m1 = PM::pm_MaxT(MinRoughness, mRoughnessX ? mRoughnessX->eval(point) : 0);
			const float m2 = PM::pm_MaxT(MinRoughness, mRoughnessY ? mRoughnessY->eval(point) : 0);

			const float NdotL = std::abs(PM::pm_Dot3D(point.N, L));

			// Since H appears to equal powers in both the numerator and denominator of the exponent, no normalization is needed.
			const PM::vec3 H = PM::pm_Subtract(L, point.V);
			const float NdotH = PM::pm_Dot3D(point.N, H);

			if (point.NdotV > PM_EPSILON && NdotL > PM_EPSILON && NdotH > PM_EPSILON)
			{
				const float HdotX = PM::pm_Dot3D(H, point.Nx);
				const float HdotY = PM::pm_Dot3D(H, point.Ny);

				const float NdotH2 = 0.5f + 0.5f * NdotH;
				const float fx = HdotX / m1;
				const float fy = HdotY / m2;
				float r = PM::pm_MagnitudeSqr3D(H) * std::exp(-(fx*fx + fy*fy) / NdotH2) * PM_INV_PI_F / (m1 * m2 * std::sqrt(NdotL*point.NdotV));

				PR_ASSERT(!std::isnan(r)/* && r >= 0 && r <= 1*/);

				if (std::isinf(r))
					r = 1;

				spec = mSpecularity->eval(point) * r;
			}
		}

		return albedo + spec;
	}

	float WardMaterial::pdf(const SamplePoint& point, const PM::vec3& L)
	{
		const float m1 = PM::pm_MaxT(MinRoughness, mRoughnessX ? mRoughnessX->eval(point) : 0);
		const float m2 = PM::pm_MaxT(MinRoughness, mRoughnessY ? mRoughnessY->eval(point) : 0);

		const float NdotL = std::abs(PM::pm_Dot3D(point.N, L));

		if (point.NdotV <= PM_EPSILON || NdotL <= PM_EPSILON)
			return PM_INV_PI_F;

		const PM::vec3 H = PM::pm_Subtract(L, point.V);

		const float NdotH = std::abs(PM::pm_Dot3D(point.N, H));

		if (NdotH <= PM_EPSILON)
			return PM_INV_PI_F;

		const float HdotX = PM::pm_Dot3D(H, point.Nx);
		const float HdotY = PM::pm_Dot3D(H, point.Ny);

		const float NdotH2 = 0.5f + 0.5f * NdotH;
		const float fx = HdotX / m1;
		const float fy = HdotY / m2;

		const float r = PM::pm_MagnitudeSqr3D(H) * std::exp(-(fx*fx + fy*fy) / NdotH2) * PM_INV_PI_F / (m1 * m2 * std::sqrt(NdotL*point.NdotV));

		PR_ASSERT(!std::isnan(r));
		if (std::isinf(r))
			return PM_INV_PI_F;
		else
			return PM::pm_MaxT(PM_INV_PI_F, r);
	}

	PM::vec3 WardMaterial::sample(const SamplePoint& point, const PM::vec3& rnd, float& pdf)
	{
		const float m1 = PM::pm_MaxT(MinRoughness, mRoughnessX ? mRoughnessX->eval(point) : 0);
		const float m2 = PM::pm_MaxT(MinRoughness, mRoughnessY ? mRoughnessY->eval(point) : 0);

		float px, py;
		if (m1 == m2)// Isotropic
		{
			px = std::atan(m1 * std::sqrt(-std::log(1 - PM::pm_GetX(rnd))));
			py = PM_2_PI_F * PM::pm_GetY(rnd);
		}
		else
		{
			float s, c;
			PM::pm_SinCosT(PM_2_PI_F * PM::pm_GetY(rnd), s, c);
			const float s2 = s*s;

			px = std::atan(std::sqrt(-std::log(1 - PM::pm_GetX(rnd)) * (m2*m2*s2 - m1*m1*s2 + m1*m1)));
			py = std::atan2(m2*s, m1*c);
		}

		auto H = Projection::tangent_align(point.N, point.Nx, point.Ny,
			Projection::sphere(px*2*PM_INV_PI_F, py*PM_INV_PI_F));
		auto dir = Reflection::reflect(PM::pm_Dot3D(H, point.V), H, point.V);
		pdf = WardMaterial::pdf(point, dir);
		return dir;
	}
}