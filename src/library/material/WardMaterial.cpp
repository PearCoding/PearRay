#include "WardMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"

#include "math/Projection.h"
#include "math/Reflection.h"
#include "math/Fresnel.h"

namespace PR
{
	WardMaterial::WardMaterial() :
		Material(), mAlbedo(nullptr), mSpecularity(nullptr), mRoughnessX(nullptr), mRoughnessY(nullptr)
	{
	}

	Texture2D* WardMaterial::albedo() const
	{
		return mAlbedo;
	}

	void WardMaterial::setAlbedo(Texture2D* diffSpec)
	{
		mAlbedo = diffSpec;
	}

	Texture2D* WardMaterial::specularity() const
	{
		return mSpecularity;
	}

	void WardMaterial::setSpecularity(Texture2D* spec)
	{
		mSpecularity = spec;
	}

	Data2D* WardMaterial::roughnessX() const
	{
		return mRoughnessX;
	}

	void WardMaterial::setRoughnessX(Data2D* d)
	{
		mRoughnessX = d;
	}

	Data2D* WardMaterial::roughnessY() const
	{
		return mRoughnessY;
	}

	void WardMaterial::setRoughnessY(Data2D* d)
	{
		mRoughnessY = d;
	}

	constexpr float MinRoughness = 0.001f;
	Spectrum WardMaterial::apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		const float NdotV = std::abs(PM::pm_Dot3D(point.normal(), V));
		const float NdotL = std::abs(PM::pm_Dot3D(point.normal(), L));

		Spectrum albedo;
		if (mAlbedo)
		{
			albedo = mAlbedo->eval(point.uv()) * PM_INV_PI_F;
		}

		Spectrum spec;
		if (mSpecularity && mRoughnessX && mRoughnessY &&
			NdotV >= PM_EPSILON && NdotL >= PM_EPSILON)
		{
			const float m1 = PM::pm_MaxT(MinRoughness, mRoughnessX ? mRoughnessX->eval(point.uv()) : 0);
			const float m2 = PM::pm_MaxT(MinRoughness, mRoughnessY ? mRoughnessY->eval(point.uv()) : 0);

			// Since H appears to equal powers in both the numerator and denominator of the exponent, no normalization is needed.
			const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Subtract(L, V));
			const float NdotH = std::abs(PM::pm_Dot3D(point.normal(), H));

			const float HdotX = PM::pm_Dot3D(H, point.tangent());
			const float HdotY = PM::pm_Dot3D(H, point.binormal());
			
			const float NdotH2 = NdotH * NdotH;
			const float fx = HdotX / m1;
			const float fy = HdotY / m2;
			const float r = std::exp(-(fx*fx + fy*fy) / NdotH2) * PM_INV_PI_F / (4 * m1 * m2 * NdotH2 * NdotH2);

			PR_ASSERT(!std::isnan(r) && !std::isinf(r)/* && r >= 0 && r <= 1*/);

			spec = mSpecularity->eval(point.uv()) * r;
		}

		return albedo + spec;
	}

	float WardMaterial::pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L)
	{
		const float NdotV = std::abs(PM::pm_Dot3D(point.normal(), V));
		const float NdotL = std::abs(PM::pm_Dot3D(point.normal(), L));

		if (NdotL <= PM_EPSILON || NdotV <= PM_EPSILON)
		{
			return 0;
		}
		else
		{
			const float m1 = PM::pm_MaxT(MinRoughness, mRoughnessX ? mRoughnessX->eval(point.uv()) : 0);
			const float m2 = PM::pm_MaxT(MinRoughness, mRoughnessY ? mRoughnessY->eval(point.uv()) : 0);

			const PM::vec3 H = PM::pm_Normalize3D(PM::pm_Subtract(L, V));

			const float NdotH = PM::pm_Dot3D(point.normal(), H);

			const float HdotX = PM::pm_Dot3D(H, point.tangent());
			const float HdotY = PM::pm_Dot3D(H, point.binormal());

			const float NdotH2 = NdotH * NdotH;
			const float fx = HdotX / m1;
			const float fy = HdotY / m2;

			const float r = std::exp(-(fx*fx + fy*fy) / NdotH2) * PM_INV_PI_F / (4 * m1 * m2 * NdotH2 * NdotH2);
			PR_ASSERT(!std::isnan(r) && !std::isinf(r));
			return r;
		}
	}

	PM::vec3 WardMaterial::sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf)
	{
		const float m1 = PM::pm_MaxT(MinRoughness, mRoughnessX ? mRoughnessX->eval(point.uv()) : 0);
		const float m2 = PM::pm_MaxT(MinRoughness, mRoughnessY ? mRoughnessY->eval(point.uv()) : 0);

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

		auto H = Projection::tangent_align(point.normal(), point.tangent(), point.binormal(),
			Projection::sphereRAD(px, py));
		auto dir = Reflection::reflect(PM::pm_Dot3D(H, V), H, V);
		pdf = WardMaterial::pdf(point, V, dir);
		return dir;
	}
}