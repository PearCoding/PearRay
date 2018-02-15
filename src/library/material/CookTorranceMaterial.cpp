#include "CookTorranceMaterial.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"

#include "shader/ConstScalarOutput.h"
#include "shader/ConstSpectralOutput.h"
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

const std::shared_ptr<SpectrumShaderOutput>& CookTorranceMaterial::albedo() const
{
	return mAlbedo;
}

void CookTorranceMaterial::setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec)
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

const std::shared_ptr<SpectrumShaderOutput>& CookTorranceMaterial::specularity() const
{
	return mSpecularity;
}

void CookTorranceMaterial::setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec)
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

const std::shared_ptr<SpectrumShaderOutput>& CookTorranceMaterial::ior() const
{
	return mIOR;
}

void CookTorranceMaterial::setIOR(const std::shared_ptr<SpectrumShaderOutput>& data)
{
	mIOR = data;
}

const std::shared_ptr<SpectrumShaderOutput>& CookTorranceMaterial::conductorAbsorption() const
{
	return mConductorAbsorption;
}

void CookTorranceMaterial::setConductorAbsorption(const std::shared_ptr<SpectrumShaderOutput>& data)
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

struct CTM_ThreadData {
	Spectrum IOR;
	Spectrum F;
	Spectrum Conductor;
	Spectrum Specularity;

	explicit CTM_ThreadData(RenderContext* context)
		: IOR(context->spectrumDescriptor())
		, F(context->spectrumDescriptor())
		, Conductor(context->spectrumDescriptor())
		, Specularity(context->spectrumDescriptor())
	{
	}
};

constexpr float MinRoughness = 0.001f;
void CookTorranceMaterial::setup(RenderContext* context)
{
	mThreadData.clear();
	for (size_t i = 0; context->threads(); ++i) {
		mThreadData.push_back(std::make_shared<CTM_ThreadData>(context));
	}

	if (!mReflectivity)
		mReflectivity = std::make_shared<ConstScalarShaderOutput>(0.5f);

	if (!mDiffuseRoughness)
		mDiffuseRoughness = std::make_shared<ConstScalarShaderOutput>(1.0f);

	if (!mAlbedo)
		mAlbedo = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::black(context->spectrumDescriptor()));

	if (!mSpecularity)
		mSpecularity = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::white(context->spectrumDescriptor()));

	if (!mIOR)
		mIOR = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::gray(context->spectrumDescriptor(), 1.55f));

	if (!mConductorAbsorption)
		mConductorAbsorption = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::black(context->spectrumDescriptor()));

	if (!mSpecRoughnessX)
		mSpecRoughnessX = (mSpecRoughnessY ? mSpecRoughnessY : std::make_shared<ConstScalarShaderOutput>(MinRoughness));

	if (!mSpecRoughnessY)
		mSpecRoughnessY = mSpecRoughnessX;
}

// Alot of potential to optimize!
void CookTorranceMaterial::eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	float refl = mReflectivity->eval(point);

	if (refl < 1) {
		float val		= PR_1_PI;
		float roughness = mDiffuseRoughness->eval(point);
		roughness *= roughness; // Square

		if (roughness > PR_EPSILON) // Oren Nayar
			val = BRDF::orennayar(roughness, point.V, point.N, L, point.NdotV, NdotL);

		mAlbedo->eval(spec, point);
		spec *= val * (1 - refl);
	}

	if (refl > PR_EPSILON && -NdotL * point.NdotV > PR_EPSILON) {
		const std::shared_ptr<CTM_ThreadData>& data = mThreadData[session.thread()];

		mIOR->eval(data->IOR, point);

		switch (mFresnelMode) {
		default:
		case FM_Dielectric:
			for (uint32 i = 0; i < data->F.samples(); ++i) {
				data->F.setValue(i, Fresnel::dielectric(-point.NdotV, 1, data->IOR.value(i)));
			}
			break;
		case FM_Conductor: {
			mConductorAbsorption->eval(data->Conductor, point);

			for (uint32 i = 0; i < data->F.samples(); ++i) {
				data->F.setValue(i, Fresnel::conductor(-point.NdotV, data->IOR.value(i), data->Conductor.value(i)));
			}
		} break;
		}

		const Eigen::Vector3f H = Reflection::halfway(point.V, L);
		const float NdotH		= point.N.dot(H);

		if (NdotH > PR_EPSILON) {
			const float m1 = std::max(MinRoughness, mSpecRoughnessX->eval(point));

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
					const float m2 = std::max(MinRoughness, mSpecRoughnessY->eval(point));

					const float XdotH = std::abs(point.Nx.dot(H));
					const float YdotH = std::abs(point.Ny.dot(H));

					D = BRDF::ndf_ggx_aniso(NdotH, XdotH, YdotH, m1, m2);
				}
				break;
			}

			// TODO: Really just clamping? A better bound would be better
			// The max clamp value is just determined by try and error.
			D = std::min(std::max(D, 0.0f), 100.0f);
			mSpecularity->eval(data->Specularity, point);
			data->Specularity *= data->F;
			data->Specularity *= (0.25f * D * G * refl);
			spec += data->Specularity;
		}
	}
}

float CookTorranceMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	const float refl = mReflectivity->eval(point);
	const float m1   = std::max(MinRoughness, mSpecRoughnessX->eval(point));

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
			const float m2 = std::max(MinRoughness, mSpecRoughnessY->eval(point));

			const float XdotH = std::abs(point.Nx.dot(H));
			const float YdotH = std::abs(point.Ny.dot(H));

			D = BRDF::ndf_ggx_aniso(NdotH, XdotH, YdotH, m1, m2);
		}
		break;
	}

	return std::min(std::max(Projection::cos_hemi_pdf(NdotL) * (1 - refl) + (0.25f * D / prod) * refl, 0.0f), 1.0f);
}

MaterialSample CookTorranceMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session)
{
	const float refl = mReflectivity->eval(point);

	MaterialSample ms;
	ms.ScatteringType = MST_DiffuseReflection; // Even specular path is diffuse, as it is not fixed like a mirror

	if (rnd(2) < refl)
		ms.L = diffuse_path(point, rnd, ms.PDF_S);
	else
		ms.L = specular_path(point, rnd, ms.PDF_S);

	ms.PDF_S *= refl;

	return ms;
}

MaterialSample CookTorranceMaterial::samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path, const RenderSession& session)
{
	MaterialSample ms;
	ms.ScatteringType = MST_DiffuseReflection;

	if (path == 0)
		ms.L = diffuse_path(point, rnd, ms.PDF_S);
	else
		ms.L = specular_path(point, rnd, ms.PDF_S);

	ms.PDF_S *= mReflectivity->eval(point); // TODO: Really?

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

	const float m1 = std::max(MinRoughness, mSpecRoughnessX->eval(point));

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
		float r2;
		float alpha2;
		if (mSpecRoughnessX == mSpecRoughnessY) {
			sinPhi = std::sin(2 * PR_PI * u);
			cosPhi = std::cos(2 * PR_PI * u);
			alpha2 = m1 * m1;
			r2	 = alpha2;
		} else {
			float m2 = std::max(MinRoughness, mSpecRoughnessY->eval(point));

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
} // namespace PR
