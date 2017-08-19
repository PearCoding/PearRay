#include "CookTorranceMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

#include "math/Fresnel.h"
#include "math/Projection.h"
#include "math/Reflection.h"

#include "BRDF.h"

#include <sstream>

namespace PR {
CookTorranceMaterial::CookTorranceMaterial(uint32 id)
	: Material(id)
	, mFresnelMode(FM_Dielectric)
	, mDistributionMode(DM_GGX)
	, mGeometryMode(GM_CookTorrance)
	, mAlbedo(nullptr)
	, mDiffuseRoughness(nullptr)
	, mSpecularity(nullptr)
	, mSpecRoughnessX(nullptr)
	, mSpecRoughnessY(nullptr)
	, mIOR(nullptr)
	, mConductorAbsorption(nullptr)
	, mReflectivity(nullptr)
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

const std::shared_ptr<SpectralShaderOutput>& CookTorranceMaterial::albedo() const
{
	return mAlbedo;
}

void CookTorranceMaterial::setAlbedo(const std::shared_ptr<SpectralShaderOutput>& diffSpec)
{
	mAlbedo = diffSpec;
}

const std::shared_ptr<ScalarShaderOutput>& CookTorranceMaterial::diffuseRoughness() const
{
	return mDiffuseRoughness;
}

void CookTorranceMaterial::setDiffuseRoughness(const std::shared_ptr<ScalarShaderOutput>& d)
{
	mDiffuseRoughness = d;
}

const std::shared_ptr<SpectralShaderOutput>& CookTorranceMaterial::specularity() const
{
	return mSpecularity;
}

void CookTorranceMaterial::setSpecularity(const std::shared_ptr<SpectralShaderOutput>& spec)
{
	mSpecularity = spec;
}

const std::shared_ptr<ScalarShaderOutput>& CookTorranceMaterial::specularRoughnessX() const
{
	return mSpecRoughnessX;
}

void CookTorranceMaterial::setSpecularRoughnessX(const std::shared_ptr<ScalarShaderOutput>& d)
{
	mSpecRoughnessX = d;
}

const std::shared_ptr<ScalarShaderOutput>& CookTorranceMaterial::specularRoughnessY() const
{
	return mSpecRoughnessY;
}

void CookTorranceMaterial::setSpecularRoughnessY(const std::shared_ptr<ScalarShaderOutput>& d)
{
	mSpecRoughnessY = d;
}

const std::shared_ptr<SpectralShaderOutput>& CookTorranceMaterial::ior() const
{
	return mIOR;
}

void CookTorranceMaterial::setIOR(const std::shared_ptr<SpectralShaderOutput>& data)
{
	mIOR = data;
}

const std::shared_ptr<SpectralShaderOutput>& CookTorranceMaterial::conductorAbsorption() const
{
	return mConductorAbsorption;
}

void CookTorranceMaterial::setConductorAbsorption(const std::shared_ptr<SpectralShaderOutput>& data)
{
	mConductorAbsorption = data;
}

const std::shared_ptr<ScalarShaderOutput>& CookTorranceMaterial::reflectivity() const
{
	return mReflectivity;
}

void CookTorranceMaterial::setReflectivity(const std::shared_ptr<ScalarShaderOutput>& d)
{
	mReflectivity = d;
}

// Alot of potential to optimize!
constexpr float MinRoughness = 0.001f;
Spectrum CookTorranceMaterial::eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
{
	const float refl = mReflectivity ? mReflectivity->eval(point) : 0.5f;

	Spectrum spec;
	if (refl < 1 && mAlbedo) {
		float val = PR_1_PI;
		if (mDiffuseRoughness) {
			float roughness = mDiffuseRoughness->eval(point);
			roughness *= roughness; // Square

			if (roughness > PR_EPSILON) // Oren Nayar
				val = BRDF::orennayar(roughness, point.V, point.N, L, point.NdotV, NdotL);
		} // else lambert

		spec = mAlbedo->eval(point) * (val * (1 - refl));
	}

	if (refl > PR_EPSILON && mSpecularity && -NdotL * point.NdotV > PR_EPSILON) {
		Spectrum ind = mIOR ? mIOR->eval(point) : Spectrum(1.55f);
		Spectrum F;
		switch (mFresnelMode) {
		default:
		case FM_Dielectric:
			for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i) {
				F.setValue(i, Fresnel::dielectric(-point.NdotV, 1, ind.value(i)));
			}
			break;
		case FM_Conductor: {
			Spectrum k = mConductorAbsorption ? mConductorAbsorption->eval(point) : Spectrum(0.0f);
			for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i) {
				F.setValue(i, Fresnel::conductor(-point.NdotV, ind.value(i), k.value(i)));
			}
		} break;
		}

		const Eigen::Vector3f H = Reflection::halfway(point.V, L);
		const float NdotH		= point.N.dot(H);

		if (NdotH > PR_EPSILON) {
			const float m1 = std::max(MinRoughness, mSpecRoughnessX ? mSpecRoughnessX->eval(point) : 0);

			float G; // Includes 1/NdotL*NdotV
			switch (mGeometryMode) {
			case GM_Implicit:
				G = BRDF::g_implicit(-point.NdotV, NdotL);
				break;
			case GM_Neumann:
				G = BRDF::g_neumann(-point.NdotV, NdotL);
				break;
			default:
			case GM_CookTorrance:
				G = BRDF::g_cooktorrance(-point.NdotV, NdotL, NdotH, -point.V.dot(H));
				break;
			case GM_Kelemen:
				G = BRDF::g_kelemen(-point.NdotV, NdotL, point.V.dot(H)); // No need for -, will be quadrated anyway
				break;
			}

			float D;
			switch (mDistributionMode) {
			case DM_Blinn:
				D = BRDF::ndf_blinn(NdotH, m1);
				break;
			case DM_Beckmann:
				D = BRDF::ndf_beckmann(NdotH, m1);
				break;
			default:
			case DM_GGX:
				if (mSpecRoughnessX == mSpecRoughnessY) {
					D = BRDF::ndf_ggx_iso(NdotH, m1);
				} else {
					const float m2 = std::max(MinRoughness, mSpecRoughnessY ? mSpecRoughnessY->eval(point) : 0);

					const float XdotH = std::abs(point.Nx.dot(H));
					const float YdotH = std::abs(point.Ny.dot(H));

					D = BRDF::ndf_ggx_aniso(NdotH, XdotH, YdotH, m1, m2);
				}
				break;
			}

			// TODO: Really just clamping? A better bound would be better
			// The max clamp value is just determined by try and error.
			D = std::min(std::max(D, 0.0f), 100.0f);
			spec += mSpecularity->eval(point) * F * (0.25f * D * G * refl);
		}
	}

	return spec;
}

float CookTorranceMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
{
	const float refl = mReflectivity ? mReflectivity->eval(point) : 0.5f;
	const float m1   = std::max(MinRoughness, mSpecRoughnessX ? mSpecRoughnessX->eval(point) : 0);

	const Eigen::Vector3f H = Reflection::halfway(point.V, L);
	const float NdotH		= point.N.dot(H);
	const float prod		= -NdotL * point.NdotV;

	if (NdotH <= PR_EPSILON || prod <= PR_EPSILON)
		return 0;

	float D;
	switch (mDistributionMode) {
	case DM_Blinn:
		D = BRDF::ndf_blinn(NdotH, m1);
		break;
	case DM_Beckmann:
		D = BRDF::ndf_beckmann(NdotH, m1);
		break;
	case DM_GGX:
	default:
		if (mSpecRoughnessX == mSpecRoughnessY) {
			D = BRDF::ndf_ggx_iso(NdotH, m1);
		} else {
			const float m2 = std::max(MinRoughness, mSpecRoughnessY ? mSpecRoughnessY->eval(point) : 0);

			const float XdotH = std::abs(point.Nx.dot(H));
			const float YdotH = std::abs(point.Ny.dot(H));

			D = BRDF::ndf_ggx_aniso(NdotH, XdotH, YdotH, m1, m2);
		}
		break;
	}

	return std::min(std::max(Projection::cos_hemi_pdf(NdotL) * (1 - refl) + (0.25f * D / prod) * refl, 0.0f), 1.0f);
}

MaterialSample CookTorranceMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd)
{
	const float refl = mReflectivity ? mReflectivity->eval(point) : 0.5f;

	MaterialSample ms;
	if (rnd(2) < refl)
		ms.L = diffuse_path(point, rnd, ms.PDF_S);
	else
		ms.L = specular_path(point, rnd, ms.PDF_S);

	return ms;
}

MaterialSample CookTorranceMaterial::samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path)
{
	MaterialSample ms;
	ms.Weight = mReflectivity ? mReflectivity->eval(point) : 0.5f;

	if (path == 0)
		ms.L = diffuse_path(point, rnd, ms.PDF_S);
	else
		ms.L = specular_path(point, rnd, ms.PDF_S);
	
	return ms;
}

uint32 CookTorranceMaterial::samplePathCount() const
{
	return 2;
}

Eigen::Vector3f CookTorranceMaterial::diffuse_path(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf)
{
	return Projection::tangent_align(point.N, point.Nx, point.Ny,
									 Projection::cos_hemi(rnd(0), rnd(1), pdf));
}

Eigen::Vector3f CookTorranceMaterial::specular_path(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf)
{
	float u = rnd(0);
	float v = rnd(1);

	float m1 = mSpecRoughnessX ? mSpecRoughnessX->eval(point) : 0;
	float cosTheta, sinTheta; // V samples
	float cosPhi, sinPhi;	 // U samples

	switch (mDistributionMode) {
	case DM_Blinn: {
		if (m1 <= PR_EPSILON)
			cosTheta = 1;
		else
			cosTheta = std::pow(1 - v, 1 / (2 * m1 * m1));

		sinPhi = std::sin(2 * PR_PI * u);
		cosPhi = std::cos(2 * PR_PI * u);

		if (m1 <= PR_EPSILON)
			pdf = 1;
		else
			pdf = 4 * PR_PI * m1 * m1 * (1 - v) / cosTheta;
	} break;
	default:
	case DM_Beckmann: {
		float t  = 1 / (1 - m1 * m1 * std::log(1 - v));
		cosTheta = std::sqrt(t);
		sinPhi   = std::sin(2 * PR_PI * u);
		cosPhi   = std::cos(2 * PR_PI * u);

		if (m1 <= PR_EPSILON)
			pdf = 1;
		else
			pdf = PR_1_PI / (m1 * m1 * cosTheta * cosTheta * cosTheta) * (1 - v);
	} break;
	case DM_GGX: {
		m1 = std::max(MinRoughness, m1);
		float r2;
		float alpha2;
		if (mSpecRoughnessX == mSpecRoughnessY) {
			sinPhi = std::sin(2 * PR_PI * u);
			cosPhi = std::cos(2 * PR_PI * u);
			alpha2 = m1 * m1;
			r2	 = alpha2;
		} else {
			const float m2 = std::max(MinRoughness, mSpecRoughnessY ? mSpecRoughnessY->eval(point) : 0);

			float phi = std::atan(m2 / m1 * std::tan(PR_PI + 2 * PR_PI * u)) + PR_PI * std::floor(2 * u + 0.5f);

			sinPhi = std::sin(phi);
			cosPhi = std::cos(phi);

			float f1 = cosPhi / m1;
			float f2 = sinPhi / m2;
			alpha2   = 1 / (f1 * f1 + f2 * f2);
			r2		 = m1 * m2;
		}

		float t2 = alpha2 * v / (1 - v);
		cosTheta = std::max(0.001f, 1.0f / std::sqrt(1 + t2));

		float s = 1 + t2 / alpha2;
		pdf		= PR_1_PI / (r2 * cosTheta * cosTheta * cosTheta * s * s);
	} break;
	}

	sinTheta = std::sqrt(1 - cosTheta * cosTheta);
	auto H   = Projection::tangent_align(point.N, point.Nx, point.Ny,
									   Eigen::Vector3f(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta));
	auto dir = Reflection::reflect(std::abs(H.dot(point.V)), H, point.V).normalized();

	float NdotL = point.N.dot(dir);
	if (NdotL > PR_EPSILON)
		pdf = std::min(std::max(pdf / (-4 * NdotL * point.NdotV), 0.0f), 1.0f);
	else
		pdf = 0;

	return dir;
}

std::string CookTorranceMaterial::dumpInformation() const
{
	std::stringstream stream;

	stream << std::boolalpha << Material::dumpInformation()
		   << "  <CookTorranceMaterial>:" << std::endl
		   << "    FresnelMode:             " << mFresnelMode << std::endl
		   << "    DistributionMode:        " << mDistributionMode << std::endl
		   << "    GeometryMode:            " << mGeometryMode << std::endl
		   << "    HasAlbedo:               " << (mAlbedo ? "true" : "false") << std::endl
		   << "    HasDiffuseRougness:      " << (mDiffuseRoughness ? "true" : "false") << std::endl
		   << "    HasSpecularity:          " << (mSpecularity ? "true" : "false") << std::endl
		   << "    HasSpecularityRougnessX: " << (mSpecRoughnessX ? "true" : "false") << std::endl
		   << "    HasSpecularityRougnessY: " << (mSpecRoughnessY ? "true" : "false") << std::endl
		   << "    HasIOR:                  " << (mIOR ? "true" : "false") << std::endl
		   << "    HasConductorAbsorption:  " << (mConductorAbsorption ? "true" : "false") << std::endl
		   << "    HasReflectivity:         " << (mReflectivity ? "true" : "false") << std::endl;

	return stream.str();
}
}
