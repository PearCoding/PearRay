#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Sampling.h"
#include "math/Tangent.h"
#include "renderer/RenderContext.h"
#include "serialization/FileSerializer.h"
#include "spectral/SpectralUpsampler.h"

#include <sstream>

namespace PR {
constexpr uint32 HalfThetaCount	 = 90;
constexpr uint32 DiffThetaCount	 = 90;
constexpr uint32 DiffPhiCount	 = 180;
constexpr uint32 MerlSampleCount = HalfThetaCount * DiffThetaCount * DiffPhiCount;

constexpr double SCALE_R = 1.00 / 1500;
constexpr double SCALE_G = 1.15 / 1500;
constexpr double SCALE_B = 1.66 / 1500;

class MerlMeasurement {
public:
	MerlMeasurement(SpectralUpsampler* upsampler, const std::filesystem::path& filename)
		: mFilename(filename)
		, mGood(false)
	{
		PR_ASSERT(upsampler, "Expected valid upsampler");

		FileSerializer serializer(filename, true);
		if (!serializer.isValid())
			return;

		// Read header
		uint32 dims[3];
		serializer.read(dims[0]);
		serializer.read(dims[1]);
		serializer.read(dims[2]);

		if (dims[0] * dims[1] * dims[2] != MerlSampleCount) {
			PR_LOG(L_ERROR) << "Given merl file '" << filename << "' does not match the dimensions" << std::endl;
			return;
		}

		if (!serializer.isValid())
			return;

		// Read content
		std::vector<double> rgb_samples(MerlSampleCount * 3);
		serializer.readRaw(reinterpret_cast<uint8*>(rgb_samples.data()), sizeof(double) * rgb_samples.size());

		if (!serializer.isValid())
			return;

		// Convert RGB to Parametric space (and double to float)
		mData.resize(MerlSampleCount);
		PR_OPT_LOOP
		for (uint32 halfThetaI = 0; halfThetaI < HalfThetaCount; ++halfThetaI) {
			for (uint32 diffThetaI = 0; diffThetaI < DiffThetaCount; ++diffThetaI) {
				for (uint32 diffPhiI = 0; diffPhiI < DiffPhiCount; ++diffPhiI) {
					uint32 sampleIndex = diffPhiI
										 + DiffPhiCount * diffThetaI
										 + DiffPhiCount * DiffThetaCount * halfThetaI;

					const float r = std::max(0.0, rgb_samples[sampleIndex + 0 * MerlSampleCount]) * SCALE_R;
					const float g = std::max(0.0, rgb_samples[sampleIndex + 1 * MerlSampleCount]) * SCALE_G;
					const float b = std::max(0.0, rgb_samples[sampleIndex + 2 * MerlSampleCount]) * SCALE_B;

					ParametricBlob blob;
					upsampler->prepare(&r, &g, &b, &blob(0), &blob(1), &blob(2), 1);
					mData[sampleIndex] = blob;
				}
			}
		}
		mGood = true;
	}

	inline const std::filesystem::path& filename() const { return mFilename; }
	bool isValid() const { return mGood; }

	SpectralBlob eval(const Vector3f& H, const Vector3f& L, const SpectralBlob& wvls) const
	{
		float theta_h = std::acos(std::max(0.0f, std::min(1.0f, H(2))));
		float theta_d = std::acos(std::max(0.0f, std::min(1.0f, H.dot(L))));

		float phi_d = 0.0f;
		if (theta_d < 1e-3f) {
			phi_d = std::atan2(-L(1), L(0)); // phi_h
		} else if (theta_h > 1e-3f) {
			/*Vector3f u, v;
			Tangent::frame(H, u, v);*/
			// use Gram-Schmidt orthonormalization to find diff basis vectors
			Vector3f u = -(Vector3f(0, 0, 1) - H(2) * H).normalized();
			Vector3f v = H.cross(u);
			phi_d	   = std::atan2(L.dot(v), L.dot(u));
		} else {
			theta_h = 0.0f;
		}

		if (phi_d < 0.0f)
			phi_d += PR_PI;

		uint32 halfThetaI = theta_h <= 0.0f ? 0 : std::min<uint32>(HalfThetaCount - 1, std::sqrt(theta_h * 2.0f * PR_INV_PI) * HalfThetaCount);
		uint32 diffThetaI = std::min<uint32>(DiffThetaCount - 1, std::max<int>(0, theta_d * 2.0f * PR_INV_PI * DiffThetaCount));
		uint32 diffPhiI	  = std::min<uint32>(DiffPhiCount - 1, std::max<int>(0, phi_d * PR_INV_PI * DiffPhiCount));

		uint32 sampleIndex = diffPhiI
							 + DiffPhiCount * diffThetaI
							 + DiffPhiCount * DiffThetaCount * halfThetaI;

		// TODO: Interpolation?
		return SpectralUpsampler::compute(mData[sampleIndex], wvls);
	}

private:
	const std::filesystem::path mFilename;
	std::vector<ParametricBlob, Eigen::aligned_allocator<ParametricBlob>> mData;
	bool mGood;
};

class MerlMeasuredMaterial : public IMaterial {
public:
	MerlMeasuredMaterial(const MerlMeasurement& measurement, const std::shared_ptr<FloatSpectralNode>& tint)
		: IMaterial()
		, mMeasurement(measurement)
		, mTint(tint)
	{
	}

	virtual ~MerlMeasuredMaterial() = default;

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const Vector3f H = Scattering::halfway_reflection(in.Context.V, in.Context.L);
		out.Weight		 = mTint->eval(in.ShadingContext) * mMeasurement.eval(H, in.Context.L, in.Context.WavelengthNM);
		out.PDF_S		 = Sampling::cos_hemi_pdf(std::max(0.0f, in.Context.NdotL()));
		out.Type		 = MST_DiffuseReflection;
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;
		out.PDF_S = Sampling::cos_hemi_pdf(std::max(0.0f, in.Context.NdotL()));
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		out.L			 = Sampling::cos_hemi(in.RND.getFloat(), in.RND.getFloat());
		const Vector3f H = Scattering::halfway_reflection(in.Context.V, out.L);

		out.Weight = mTint->eval(in.ShadingContext) * mMeasurement.eval(H, out.L, in.Context.WavelengthNM);
		out.Type   = MST_DiffuseReflection;
		out.PDF_S  = Sampling::cos_hemi_pdf(out.L(2));
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <MerlMeasuredMaterial>:" << std::endl
			   << "    Filename: " << mMeasurement.filename() << std::endl
			   << "    Tint:     " << mTint << std::endl;

		return stream.str();
	}

private:
	const MerlMeasurement mMeasurement;
	const std::shared_ptr<FloatSpectralNode> mTint;
};

class MerlMeasuredMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();
		MerlMeasurement measurement(ctx.environment()->defaultSpectralUpsampler().get(), ctx.escapePath(params.getString("filename", "")));

		if (measurement.isValid())
			return std::make_shared<MerlMeasuredMaterial>(measurement,
														  ctx.lookupSpectralNode("tint", 1));
		else
			return nullptr;
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "merl", "merl_measured", "merl-measured" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::MerlMeasuredMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)