#include "WardMaterial.h"
#include "ray/Ray.h"

#include "renderer/RenderContext.h"
#include "renderer/RenderSession.h"

#include "shader/ConstScalarOutput.h"
#include "shader/ConstSpectralOutput.h"
#include "shader/ShaderClosure.h"

#include "math/Fresnel.h"
#include "math/Projection.h"
#include "math/Reflection.h"

#include <sstream>

namespace PR {
WardMaterial::WardMaterial(uint32 id)
	: Material(id)
	, mAlbedo(nullptr)
	, mSpecularity(nullptr)
	, mRoughnessX(nullptr)
	, mRoughnessY(nullptr)
	, mReflectivity(nullptr)
{
}

std::shared_ptr<SpectrumShaderOutput> WardMaterial::albedo() const
{
	return mAlbedo;
}

void WardMaterial::setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec)
{
	mAlbedo = diffSpec;
}

std::shared_ptr<SpectrumShaderOutput> WardMaterial::specularity() const
{
	return mSpecularity;
}

void WardMaterial::setSpecularity(const std::shared_ptr<SpectrumShaderOutput>& spec)
{
	mSpecularity = spec;
}

std::shared_ptr<ScalarShaderOutput> WardMaterial::roughnessX() const
{
	return mRoughnessX;
}

void WardMaterial::setRoughnessX(const std::shared_ptr<ScalarShaderOutput>& d)
{
	mRoughnessX = d;
}

std::shared_ptr<ScalarShaderOutput> WardMaterial::roughnessY() const
{
	return mRoughnessY;
}

void WardMaterial::setRoughnessY(const std::shared_ptr<ScalarShaderOutput>& d)
{
	mRoughnessY = d;
}

std::shared_ptr<ScalarShaderOutput> WardMaterial::reflectivity() const
{
	return mReflectivity;
}

void WardMaterial::setReflectivity(const std::shared_ptr<ScalarShaderOutput>& d)
{
	mReflectivity = d;
}

struct WM_ThreadData {
	Spectrum Albedo;

	explicit WM_ThreadData(RenderContext* context)
		: Albedo(context->spectrumDescriptor())
	{
	}
};

constexpr float MinRoughness = 0.001f;
void WardMaterial::setup(RenderContext* context)
{
	mThreadData.clear();
	for (size_t i = 0; i < context->threads(); ++i)
		mThreadData.push_back(std::make_shared<WM_ThreadData>(context));

	if (!mAlbedo)
		mSpecularity = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::black(context->spectrumDescriptor()));

	if (!mSpecularity)
		mSpecularity = std::make_shared<ConstSpectrumShaderOutput>(Spectrum::white(context->spectrumDescriptor()));

	if (!mReflectivity)
		mReflectivity = std::make_shared<ConstScalarShaderOutput>(0.5f);

	if (!mRoughnessX)
		mRoughnessX = mRoughnessY ? mRoughnessY : std::make_shared<ConstScalarShaderOutput>(0.0f);

	if (!mRoughnessY)
		mRoughnessY = mRoughnessX;
}

void WardMaterial::eval(Spectrum& spec, const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	const float refl	= mReflectivity->eval(point);
	const std::shared_ptr<WM_ThreadData>& data = mThreadData[session.thread()];

	mAlbedo->eval(data->Albedo, point);
	data->Albedo *= PR_1_PI;

	const float m1 = std::max(MinRoughness, mRoughnessX->eval(point));
	const float m2 = std::max(MinRoughness, mRoughnessY->eval(point));

	const Eigen::Vector3f H = Reflection::halfway(point.V, L);
	const float NdotH		= point.N.dot(H);
	const float prod		= -NdotL * point.NdotV;

	if (NdotH > PR_EPSILON && prod > PR_EPSILON) {
		const float HdotX = std::abs(H.dot(point.Nx));
		const float HdotY = std::abs(H.dot(point.Ny));

		const float NdotH2 = 0.5f + 0.5f * NdotH;
		const float fx	 = HdotX / m1;
		const float fy	 = HdotY / m2;
		float r			   = std::exp(-(fx * fx + fy * fy) / NdotH2) * PR_1_PI / (4 * m1 * m2 * std::sqrt(prod));

		mSpecularity->eval(spec, point);
		spec *= std::min(r, 1.0f);
	}

	spec *= refl;
	spec += data->Albedo * (1 - refl);
}

float WardMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL, const RenderSession& session)
{
	const float refl = mReflectivity->eval(point);
	const float m1   = std::max(MinRoughness, mRoughnessX->eval(point));
	const float m2   = std::max(MinRoughness, mRoughnessY->eval(point));

	const Eigen::Vector3f H = Reflection::halfway(point.V, L);
	const float NdotH		= point.N.dot(H);
	const float prod		= -NdotL * point.NdotV;

	if (NdotH <= PR_EPSILON || prod <= PR_EPSILON)
		return PR_1_PI;

	const float HdotX = std::abs(H.dot(point.Nx));
	const float HdotY = std::abs(H.dot(point.Ny));

	const float NdotH2 = 0.5f + 0.5f * NdotH;
	const float fx	 = HdotX / m1;
	const float fy	 = HdotY / m2;

	const float r = std::exp(-(fx * fx + fy * fy) / NdotH2) * PR_1_PI / (4 * m1 * m2 * std::sqrt(prod));

	return std::min(std::max(Projection::cos_hemi_pdf(NdotL) * (1 - refl) + r * refl, 0.0f), 1.0f);
}

MaterialSample WardMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, const RenderSession& session)
{
	const float refl = mReflectivity->eval(point);

	MaterialSample ms;
	ms.ScatteringType = MST_DiffuseReflection;

	if (rnd(2) < refl)
		ms.L = diffuse_path(point, rnd, ms.PDF_S);
	else
		ms.L = specular_path(point, rnd, ms.PDF_S);

	ms.PDF_S *= refl;
	return ms;
}

MaterialSample WardMaterial::samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path, const RenderSession& session)
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

uint32 WardMaterial::samplePathCount() const
{
	return 2;
}

Eigen::Vector3f WardMaterial::diffuse_path(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf)
{
	return Projection::tangent_align(point.N, point.Nx, point.Ny,
									 Projection::cos_hemi(rnd(0), rnd(1), pdf));
}

Eigen::Vector3f WardMaterial::specular_path(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf)
{
	float u = rnd(0);
	float v = rnd(1);

	const float m1 = std::max(MinRoughness, mRoughnessX->eval(point));

	float cosTheta, sinTheta;		// V samples
	float cosPhi, sinPhi;			// U samples
	if (mRoughnessX == mRoughnessY) // Isotropic
	{
		sinPhi = std::sin(2 * PR_PI * u);
		cosPhi = std::cos(2 * PR_PI * u);

		const float f = -std::log(std::max(0.00001f, v)) * m1 * m1;
		cosTheta	  = 1 / (1 + f);
		sinTheta	  = std::sqrt(f) * cosTheta;

		const float t = 4 * PR_PI * m1 * m1 * cosTheta * cosTheta * cosTheta * v;
		if (t <= PR_EPSILON)
			pdf = 1;
		else
			pdf = 1 / t;
	} else {
		const float m2 = std::max(MinRoughness, mRoughnessY->eval(point));

		const float pm1 = std::max(MinRoughness, m1 * m1);
		const float pm2 = std::max(MinRoughness, m2 * m2);

		const float f1 = (m2 / m1) * std::tan(2 * PR_PI * u);
		cosPhi		   = 1 / std::sqrt(1 + f1 * f1);
		sinPhi		   = f1 * cosPhi;

		const float cosPhi2 = cosPhi * cosPhi;
		const float tz		= (cosPhi2 / pm1 + sinPhi * sinPhi / pm2);
		const float f2		= -std::log(std::max(0.000001f, v)) / tz;
		cosTheta			= 1 / (1 + f2);
		sinTheta			= std::sqrt(f2) * cosTheta;

		const float cosTheta2 = cosTheta * cosTheta;
		const float tu		  = pm1 * sinPhi * sinPhi + pm2 * cosPhi2;
		const float tb		  = 4 * PR_PI * m1 * m2 * (pm1 * (1 - cosPhi2) / cosPhi + pm2 * cosPhi) * cosTheta2;
		pdf					  = tu / tb * std::exp(-tz * (1 - cosTheta2) / (cosTheta2));
	}

	auto H   = Projection::tangent_align(point.N, point.Nx, point.Ny,
										 Eigen::Vector3f(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta));
	auto dir = Reflection::reflect(std::abs(H.dot(point.V)), H, point.V);

	pdf = std::min(std::max(pdf / std::abs(point.V.dot(dir)), 0.0f), 1.0f);

	return dir;
}

std::string WardMaterial::dumpInformation() const
{
	std::stringstream stream;

	stream << std::boolalpha << Material::dumpInformation()
		   << "  <WardMaterial>:" << std::endl
		   << "    HasAlbedo:       " << (mAlbedo ? "true" : "false") << std::endl
		   << "    HasSpecularity:  " << (mSpecularity ? "true" : "false") << std::endl
		   << "    HasRoughnessX:   " << (mRoughnessX ? "true" : "false") << std::endl
		   << "    HasRoughnessY:   " << (mRoughnessY ? "true" : "false") << std::endl
		   << "    HasReflectivity: " << (mReflectivity ? "true" : "false") << std::endl;

	return stream.str();
}
} // namespace PR
