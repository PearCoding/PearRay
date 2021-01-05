#include "Environment.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "math/Fresnel.h"
#include "math/Microfacet.h"
#include "math/Projection.h"
#include "math/Sampling.h"
#include "math/Scattering.h"
#include "math/Spherical.h"
#include "spectral/CIE.h"

#include "math/MicrofacetReflection.h"
#include "math/MicrofacetTransmission.h"

#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

/* Based on the paper:
	BURLEY, Brent; STUDIOS, Walt Disney Animation. Physically-based shading at disney. In: ACM SIGGRAPH. 2012. S. 1-7.
	and code:
	https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
  And based on the follow-up paper:
    BURLEY, Brent; STUDIOS, Walt Disney Animation. Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering. (2015).
	https://blog.selfshadow.com/publications/s2015-shading-course/#course_content
*/

constexpr float EVAL_EPS = 1e-4f;
constexpr float AIR		 = 1.0002926f;
template <bool UseVNDF, bool Thin, bool HasTransmission>
struct PrincipledClosure {
	using RoughnessFunction = RoughDistribution<true, UseVNDF>;

	template <typename T>
	static inline T mix(const T& v0, const T& v1, float t)
	{
		return (1 - t) * v0 + t * v1;
	}

	inline static float schlickR0(float eta)
	{
		const float factor = (eta - 1.0f) / (eta + 1.0f);
		return factor * factor;
	}

	const SpectralBlob Base;
	const SpectralBlob IOR;
	const float DiffuseTransmission;
	const float Roughness;
	const float Anisotropic;
	const float SpecularTransmission;
	const float SpecularTint;
	const float Flatness;
	const float Metallic;
	const float Sheen;
	const float SheenTint;
	const float Clearcoat;
	const float ClearcoatGloss;

	PrincipledClosure(const SpectralBlob& base, const SpectralBlob& ior,
					  float diffuseTransmission, float roughness, float anisotropic,
					  float specularTransmission, float specularTint,
					  float flatness, float metallic,
					  float sheen, float sheenTint,
					  float clearcoat, float clearcoatGloss)
		: Base(base)
		, IOR(ior)
		, DiffuseTransmission(HasTransmission ? diffuseTransmission : 0.0f)
		, Roughness(roughness)
		, Anisotropic(anisotropic)
		, SpecularTransmission(HasTransmission ? specularTransmission : 0.0f)
		, SpecularTint(specularTint)
		, Flatness(flatness)
		, Metallic(metallic)
		, Sheen(sheen)
		, SheenTint(sheenTint)
		, Clearcoat(clearcoat)
		, ClearcoatGloss(clearcoatGloss)
	{
	}

	inline float thinTransmissionRoughness() const
	{
		return std::max(0.0f, std::min(1.0f, (0.65f * IOR.mean() - 0.35f) * Roughness));
	}

	inline RoughnessFunction roughnessClosure(float r) const
	{
		const float aspect = std::sqrt(1 - Anisotropic * 0.9f);
		const float ax	   = std::max(0.001f, r * r / aspect);
		const float ay	   = std::max(0.001f, r * r * aspect);
		return RoughnessFunction(ax, ay);
	}

	inline bool isDelta() const
	{
		return roughnessClosure(Roughness).isDelta();
	}

	struct LobeDistribution {
		float DiffuseReflection;
		float DiffuseTransmission;
		float SpecularReflection;
		float SpecularTransmission;
	};

	inline LobeDistribution calculateLobeDistribution(const ShadingVector& V) const
	{
		LobeDistribution distribution;

		distribution.DiffuseReflection	= Roughness * Roughness * (1.0f - Metallic) * (1.0f - SpecularTransmission);
		distribution.SpecularReflection = 1;

		if constexpr (HasTransmission) {
			const float F					  = Fresnel::dielectric(V.cosTheta(), AIR, IOR[0]); // Sample based on shading normal
			distribution.DiffuseTransmission  = DiffuseTransmission * distribution.DiffuseReflection;
			distribution.SpecularTransmission = (1.0f - F) * (1.0f - Metallic) * SpecularTransmission;
			distribution.SpecularReflection *= F;
		} else {
			PR_UNUSED(V);
			distribution.DiffuseTransmission  = 0;
			distribution.SpecularTransmission = 0;
		}

		const float norm = distribution.DiffuseReflection + distribution.SpecularReflection + distribution.DiffuseTransmission + distribution.SpecularTransmission;
		if (norm <= PR_EPSILON)
			return LobeDistribution{ 1.0f, 0.0f, 0.0f, 0.0f };

		distribution.DiffuseReflection /= norm;
		distribution.SpecularReflection /= norm;
		distribution.DiffuseTransmission /= norm;
		distribution.SpecularTransmission /= norm;

		return distribution;
	}

	inline SpectralBlob fresnelTerm(float dot) const
	{
		SpectralBlob res;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			res[i] = Fresnel::dielectric(dot, AIR, IOR[i]);

		return res;
	}

	inline SpectralBlob disneyFresnelTerm(float HdotV, float HdotL, const SpectralBlob& wvl) const
	{
		if (Metallic <= EVAL_EPS)
			return fresnelTerm(HdotV);

		const SpectralBlob color = tintColor(wvl);
		const SpectralBlob eta	 = HdotV < 0 ? (AIR / IOR).eval() : (IOR / AIR).eval();

		SpectralBlob res;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const float r0 = mix(schlickR0(eta[i]) * mix(1.0f, color[i], SpecularTint), Base[i], Metallic);
			const float f1 = Fresnel::dielectric(HdotV, AIR, IOR[i]);
			const float f2 = Fresnel::schlick(std::abs(HdotL), r0);

			res[i] = mix<float>(f1, f2, Metallic);
		}
		return res;
	}

	inline SpectralBlob tintColor(const SpectralBlob& wvl) const
	{
		float lum = 0;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
			lum = std::max(lum, Base[i] * CIE::eval_y(wvl[i]));

		return lum > PR_EPSILON ? SpectralBlob(Base / lum) : SpectralBlob::Ones();
	}

	inline SpectralBlob sheenTintColor(const SpectralBlob& wvl) const
	{
		return mix<SpectralBlob>(SpectralBlob::Ones(), tintColor(wvl), SheenTint);
	}

	inline float retroDiffuseTerm(const MaterialEvalContext& ctx, float HdotL) const
	{
		const float alpha2 = Roughness * Roughness;
		const float fd90   = 0.5f + 2 * HdotL * HdotL * alpha2;
		const float lk	   = Fresnel::schlick_term(ctx.L.absCosTheta());
		const float vk	   = Fresnel::schlick_term(ctx.V.absCosTheta());

		return PR_INV_PI * fd90 * (lk + vk + lk * vk * (fd90 - 1.0f));
	}

	inline float subsurfaceTerm(const MaterialEvalContext& ctx, float HdotL) const
	{
		const float alpha2 = Roughness * Roughness;
		const float fss90  = HdotL * HdotL * alpha2;
		const float lk	   = Fresnel::schlick_term(ctx.L.absCosTheta());
		const float vk	   = Fresnel::schlick_term(ctx.V.absCosTheta());

		const float fss = mix(1.0f, fss90, lk) * mix(1.0f, fss90, vk);

		const float f = ctx.L.absCosTheta() + ctx.V.absCosTheta();
		if (std::abs(f) < PR_EPSILON)
			return 0.0f;
		else
			return 1.25f * (fss * (1.0f / f - 0.5f) + 0.5f);
	}

	/// Calculate diffuse term which lambert reflection or drop-in replacement of approx subsurface scattering
	inline float diffuseTerm(const MaterialEvalContext& ctx, float HdotL) const
	{
		PR_UNUSED(HdotL); // Necessary to prevent a compiler warning

		const float lk = Fresnel::schlick_term(ctx.L.absCosTheta());
		const float vk = Fresnel::schlick_term(ctx.V.absCosTheta());

		float diffuse = 1;
		if constexpr (Thin)
			diffuse = mix(1.0f, subsurfaceTerm(ctx, HdotL), Flatness);

		return PR_INV_PI * diffuse * (1 - 0.5f * lk) * (1 - 0.5f * vk);
	}

	inline SpectralBlob specularReflectionTerm(const MaterialEvalContext& ctx, const ShadingVector& H) const
	{
		const auto micro = MicrofacetReflection(roughnessClosure(Roughness));

		const float HdotV	 = ctx.V.dot(H);
		const float HdotL	 = ctx.L.dot(H);
		const SpectralBlob F = disneyFresnelTerm(HdotV, HdotL, ctx.WavelengthNM);

		return F * micro.eval(ctx.V, ctx.L);
	}

	inline float specularRefractionTermComponent(size_t i, const MaterialEvalContext& ctx) const
	{
		const float scaledR = Thin ? thinTransmissionRoughness() : Roughness;
		const auto micro	= MicrofacetTransmission(roughnessClosure(scaledR), AIR, IOR[i]);
		const float R		= micro.evalDielectric(ctx.V, ctx.L);

		if constexpr (Thin)
			return std::sqrt(Base[i]) * R;
		else
			return Base[i] * R;
	}

	inline float clearcoatTerm(const MaterialEvalContext& ctx, const ShadingVector& H) const
	{
		// This is fixed by definition
		static float F0 = 0.04f; // IOR 1.5
		static float R	= 0.25f;

		const float D  = Microfacet::ndf_ggx(H, mix(0.1f, 0.001f, ClearcoatGloss));
		const float hk = Fresnel::schlick_term(std::abs(H.dot(ctx.L)));
		const float F  = mix(F0, 1.0f, hk);
		const float G  = Microfacet::g_1_smith_opt(ctx.L.absCosTheta(), R) * Microfacet::g_1_smith_opt(ctx.V.absCosTheta(), R);

		// 1/(4*NdotV*NdotL) already multiplied out
		return R * D * F * G;
	}

	inline SpectralBlob sheenTerm(float HdotL, const SpectralBlob& wvl) const
	{
		if (Sheen <= EVAL_EPS)
			return SpectralBlob::Zero();

		const SpectralBlob sheenColor = sheenTintColor(wvl);
		return Sheen * sheenColor * Fresnel::schlick_term(std::abs(HdotL));
	}

	inline SpectralBlob eval(const MaterialEvalContext& ctx) const
	{
		if (PR_UNLIKELY(ctx.V.absCosTheta() <= PR_EPSILON || ctx.L.absCosTheta() <= PR_EPSILON))
			return SpectralBlob::Zero();

		const float diffuseWeight = (1.0f - Metallic) * (1.0f - SpecularTransmission);

		const bool isTransmission  = !ctx.V.sameHemisphere(ctx.L);
		const bool upperHemisphere = ctx.V.cosTheta() >= 0.0f && !isTransmission;

		if constexpr (!HasTransmission) {
			if (isTransmission)
				return SpectralBlob::Zero();
		}

		const Vector3f rH = Scattering::halfway_reflection(ctx.V, ctx.L);
		const float HdotL = rH.dot((Vector3f)ctx.L);

		SpectralBlob value = SpectralBlob::Zero();

		if (diffuseWeight > EVAL_EPS) {
			// Retro + Sheen
			if (!isTransmission) {
				const float retro		 = retroDiffuseTerm(ctx, HdotL) * diffuseWeight;
				const SpectralBlob sheen = sheenTerm(HdotL, ctx.WavelengthNM) * diffuseWeight;
				value += (retro * Base + sheen) * ctx.L.absCosTheta();
			}

			// Diffuse Reflection
			if (!isTransmission) {
				const float diff = diffuseTerm(ctx, HdotL) * (Thin ? 1 - DiffuseTransmission : diffuseWeight);
				value += Base * (diff * ctx.L.absCosTheta());
			}

			// Diffuse Transmission
			if constexpr (HasTransmission && Thin) {
				if (isTransmission) {
					const float diff = diffuseTerm(ctx, HdotL) * DiffuseTransmission;
					value += Base * (diff * ctx.L.absCosTheta());
				}
			}
		}

		// Specular Reflection
		value += specularReflectionTerm(ctx, rH);

		// Specular Refraction
		if constexpr (HasTransmission) {
			const float transmissionWeight = (1.0f - Metallic) * SpecularTransmission;
			if (transmissionWeight > EVAL_EPS) {
				SpectralBlob weight;
				for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i)
					weight[i] = specularRefractionTermComponent(i, ctx);

				// Only rays from lights are weighted by this factor
				// as radiance flows in the opposite direction
				if (ctx.RayFlags & RayFlag::Light) {
					const SpectralBlob eta = HdotL < 0.0f ? (IOR / AIR).eval() : (AIR / IOR).eval();
					weight *= eta * eta;
				}

				value += transmissionWeight * weight;
			}
		}

		// Clearcoat
		if (upperHemisphere && Clearcoat > EVAL_EPS)
			value += SpectralBlob(clearcoatTerm(ctx, rH));

		return value;
	}

	inline SpectralBlob pdfDiffuse(const MaterialEvalContext& ctx) const
	{
		return SpectralBlob(Sampling::cos_hemi_pdf(ctx.L.absCosTheta()));
	}

	inline SpectralBlob pdfReflection(const MaterialEvalContext& ctx) const
	{
		const MicrofacetReflection refl = MicrofacetReflection(roughnessClosure(Roughness));
		return SpectralBlob(refl.pdf(ctx.V, ctx.L));
	}

	inline SpectralBlob pdfRefractive(const MaterialEvalContext& ctx) const
	{
		const auto roughness = roughnessClosure(Roughness);

		SpectralBlob pdf;
		PR_UNROLL_LOOP(PR_SPECTRAL_BLOB_SIZE)
		for (size_t i = 0; i < PR_SPECTRAL_BLOB_SIZE; ++i) {
			const MicrofacetTransmission refr = MicrofacetTransmission(roughness, AIR, IOR[i]);
			pdf[i]							  = refr.pdf(ctx.V, ctx.L);
		}
		return pdf;
	}

	inline SpectralBlob pdf(const MaterialEvalContext& ctx) const
	{
		if (PR_UNLIKELY(ctx.V.absCosTheta() <= PR_EPSILON || ctx.L.absCosTheta() <= PR_EPSILON))
			return SpectralBlob::Zero();

		const LobeDistribution distr = calculateLobeDistribution(ctx.V);

		const bool isTransmission  = !ctx.V.sameHemisphere(ctx.L);
		const SpectralBlob diffPdf = pdfDiffuse(ctx);

		SpectralBlob pdfV = SpectralBlob::Zero();

		if (!isTransmission) {
			pdfV += distr.DiffuseReflection * diffPdf;
			if (distr.SpecularReflection > EVAL_EPS)
				pdfV += distr.SpecularReflection * pdfReflection(ctx);
		}

		if constexpr (HasTransmission) {
			if (isTransmission) {
				pdfV += distr.DiffuseTransmission * diffPdf;
				if (distr.SpecularTransmission > EVAL_EPS)
					pdfV += distr.SpecularTransmission * pdfRefractive(ctx);
			}
		}

		return pdfV;
	}

	inline Vector3f sampleDiffuse(Random& rnd, const MaterialSampleContext& ctx) const
	{
		const bool flip	 = ctx.V.cosTheta() < 0;
		const Vector3f L = Sampling::cos_hemi(rnd.getFloat(), rnd.getFloat());
		return flip ? -L : L;
	}

	inline Vector3f sampleReflection(Random& rnd, const MaterialSampleContext& ctx) const
	{
		const MicrofacetReflection refl = MicrofacetReflection(roughnessClosure(Roughness));
		return refl.sample(rnd.get2D(), ctx.V);
	}

	inline Vector3f sampleRefractive(Random& rnd, const MaterialSampleContext& ctx) const
	{
		const MicrofacetTransmission refr = MicrofacetTransmission(roughnessClosure(Roughness), AIR, IOR[0] /* Hero wavelength */);
		return refr.sample(rnd.get2D(), ctx.V);
	}

	inline Vector3f sample(Random& rnd, const MaterialSampleContext& ctx) const
	{
		if (PR_UNLIKELY(ctx.V.absCosTheta() <= PR_EPSILON))
			return Vector3f::Zero();

		const LobeDistribution distr = calculateLobeDistribution(ctx.V);

		const float u0 = rnd.getFloat();

		if (u0 < distr.DiffuseReflection)
			return sampleDiffuse(rnd, ctx);
		else if (u0 < distr.DiffuseReflection + distr.DiffuseTransmission)
			return -sampleDiffuse(rnd, ctx);
		else if (u0 < distr.DiffuseReflection + distr.DiffuseTransmission + distr.SpecularTransmission)
			return sampleRefractive(rnd, ctx);
		else
			return sampleReflection(rnd, ctx);
	}
};

template <bool UseVNDF, bool Thin, bool HasTransmission>
class PrincipledMaterial : public IMaterial {
public:
	using EvalClosure = PrincipledClosure<UseVNDF, Thin, HasTransmission>;

	PrincipledMaterial(const std::shared_ptr<FloatSpectralNode>& baseColor,
					   const std::shared_ptr<FloatSpectralNode>& ior,
					   const std::shared_ptr<FloatScalarNode>& diffTrans,
					   const std::shared_ptr<FloatScalarNode>& roughness,
					   const std::shared_ptr<FloatScalarNode>& anisotropic,
					   const std::shared_ptr<FloatScalarNode>& specTrans,
					   const std::shared_ptr<FloatScalarNode>& specTint,
					   const std::shared_ptr<FloatScalarNode>& flatness,
					   const std::shared_ptr<FloatScalarNode>& metallic,
					   const std::shared_ptr<FloatScalarNode>& sheen,
					   const std::shared_ptr<FloatScalarNode>& sheenTint,
					   const std::shared_ptr<FloatScalarNode>& clearcoat,
					   const std::shared_ptr<FloatScalarNode>& clearcoatGloss)
		: IMaterial()
		, mBaseColor(baseColor)
		, mIOR(ior)
		, mDiffuseTransmission(diffTrans)
		, mRoughness(roughness)
		, mAnisotropic(anisotropic)
		, mSpecularTransmission(specTrans)
		, mSpecularTint(specTint)
		, mFlatness(flatness)
		, mMetallic(metallic)
		, mSheen(sheen)
		, mSheenTint(sheenTint)
		, mClearcoat(clearcoat)
		, mClearcoatGloss(clearcoatGloss)
	{
	}

	virtual ~PrincipledMaterial() = default;

	inline EvalClosure createClosure(const ShadingContext& sctx) const
	{
		const SpectralBlob base			 = mBaseColor->eval(sctx);
		const SpectralBlob ior			 = mIOR->eval(sctx);
		const float diffTrans			 = mDiffuseTransmission->eval(sctx);
		const float roughness			 = mRoughness->eval(sctx);
		const float anisotropic			 = mAnisotropic->eval(sctx);
		const float flatness			 = mFlatness->eval(sctx);
		const float metallic			 = mMetallic->eval(sctx);
		const float specularTransmission = mSpecularTransmission->eval(sctx);
		const float specularTint		 = mSpecularTint->eval(sctx);
		const float sheen				 = mSheen->eval(sctx);
		const float sheenTint			 = mSheenTint->eval(sctx);
		const float clearcoat			 = mClearcoat->eval(sctx);
		const float clearcoatGloss		 = mClearcoatGloss->eval(sctx);

		return EvalClosure(base, ior, diffTrans, roughness, anisotropic,
						   specularTransmission, specularTint,
						   flatness, metallic,
						   sheen, sheenTint,
						   clearcoat, clearcoatGloss);
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure = createClosure(in.ShadingContext);
		if (closure.isDelta()) { // Reject
			out.Weight = 0;
			out.PDF_S  = 0;
			out.Flags  = MaterialScatter::DeltaDistribution;
			return;
		}

		// Set type based on sampling result
		if (in.Context.V.sameHemisphere(in.Context.L)) {
			if (closure.Roughness < 0.5f)
				out.Type = MaterialScatteringType::SpecularReflection;
			else
				out.Type = MaterialScatteringType::DiffuseReflection;
		} else {
			if (closure.Roughness < 0.5f)
				out.Type = MaterialScatteringType::SpecularTransmission;
			else
				out.Type = MaterialScatteringType::DiffuseTransmission;
		}

		out.Weight = closure.eval(in.Context);
		out.PDF_S  = closure.pdf(in.Context);

		PR_ASSERT(out.PDF_S[0] >= 0.0f, "PDF has to be positive");
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		// Quite costly, as most stuff is not really needed
		const auto closure = createClosure(in.ShadingContext);
		if (closure.isDelta()) { // Reject
			out.PDF_S = 0;
			out.Flags = MaterialScatter::DeltaDistribution;
			return;
		}

		out.PDF_S = closure.pdf(in.Context);

		PR_ASSERT(out.PDF_S[0] >= 0.0f, "PDF has to be positive");
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const auto closure = createClosure(in.ShadingContext);
		out.L			   = closure.sample(in.RND, in.Context);

		// Set flags
		if (closure.isDelta())
			out.Flags |= MaterialScatter::DeltaDistribution;

		if (PR_UNLIKELY(out.L.isZero())) {
			out = MaterialSampleOutput::Reject(MaterialScatteringType::DiffuseReflection, out.Flags);
			return;
		}

		// Set type based on sampling result (TODO)
		if (in.Context.V.sameHemisphere(out.L)) {
			if (closure.Roughness < 0.5f)
				out.Type = MaterialScatteringType::SpecularReflection;
			else
				out.Type = MaterialScatteringType::DiffuseReflection;
		} else {
			if (closure.Roughness < 0.5f)
				out.Type = MaterialScatteringType::SpecularTransmission;
			else
				out.Type = MaterialScatteringType::DiffuseTransmission;
		}

		const auto ectx = in.Context.expand(out.L);
		out.Weight		= closure.eval(ectx);
		out.PDF_S		= closure.pdf(ectx);

		// If we handle a delta case, make sure the outgoing pdf will be 1
		if (closure.isDelta() && out.PDF_S[0] > PR_EPSILON) {
			out.Weight /= out.PDF_S[0];
			out.PDF_S = 1.0f;
		}

		PR_ASSERT(out.PDF_S[0] >= 0.0f, "PDF has to be positive");
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <PrincipledMaterial>:" << std::endl
			   << "    BaseColor:            " << mBaseColor->dumpInformation() << std::endl
			   << "    IOR:                  " << mIOR->dumpInformation() << std::endl
			   << "    DiffuseTransmission:  " << mDiffuseTransmission->dumpInformation() << std::endl
			   << "    SpecularTransmission: " << mSpecularTransmission->dumpInformation() << std::endl
			   << "    SpecularTint:         " << mSpecularTint->dumpInformation() << std::endl
			   << "    Roughness:            " << mRoughness->dumpInformation() << std::endl
			   << "    Anisotropic:          " << mAnisotropic->dumpInformation() << std::endl
			   << "    Flatness:             " << mFlatness->dumpInformation() << std::endl
			   << "    Metallic:             " << mMetallic->dumpInformation() << std::endl
			   << "    Sheen:                " << mSheen->dumpInformation() << std::endl
			   << "    SheenTint:            " << mSheenTint->dumpInformation() << std::endl
			   << "    Clearcoat:            " << mClearcoat->dumpInformation() << std::endl
			   << "    ClearcoatGloss:       " << mClearcoatGloss->dumpInformation() << std::endl
			   << "    Thin:                 " << (Thin ? "true" : "false") << std::endl
			   << "    VNDF:                 " << (UseVNDF ? "true" : "false") << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralNode> mBaseColor;
	std::shared_ptr<FloatSpectralNode> mIOR;
	std::shared_ptr<FloatScalarNode> mDiffuseTransmission;
	std::shared_ptr<FloatScalarNode> mRoughness;
	std::shared_ptr<FloatScalarNode> mAnisotropic;
	std::shared_ptr<FloatScalarNode> mSpecularTransmission;
	std::shared_ptr<FloatScalarNode> mSpecularTint;
	std::shared_ptr<FloatScalarNode> mFlatness;
	std::shared_ptr<FloatScalarNode> mMetallic;
	std::shared_ptr<FloatScalarNode> mSheen;
	std::shared_ptr<FloatScalarNode> mSheenTint;
	std::shared_ptr<FloatScalarNode> mClearcoat;
	std::shared_ptr<FloatScalarNode> mClearcoatGloss;
}; // namespace PR

template <bool UseVNDF, bool Thin, bool HasTransmission>
inline static std::shared_ptr<IMaterial> createPrincipled1(const SceneLoadContext& ctx)
{
	const auto base_color	   = ctx.lookupSpectralNode({ "base_color", "base" }, 0.8f);
	const auto ior			   = ctx.lookupSpectralNode({ "ior", "eta", "index" }, 1.55f);
	const auto diffuse_trans   = ctx.lookupScalarNode({ "diffuse_transmission", "diff_trans" }, 0.0f);
	const auto specular_trans  = ctx.lookupScalarNode({ "specular_transmission", "spec_trans" }, 0.0f);
	const auto specular_tint   = ctx.lookupScalarNode("specular_tint", 0.0f);
	const auto roughness	   = ctx.lookupScalarNode("roughness", 0.5f);
	const auto anisotropic	   = ctx.lookupScalarNode("anisotropic", 0.0f);
	const auto flatness		   = ctx.lookupScalarNode({ "flatness", "subsurface" }, 0.0f);
	const auto metallic		   = ctx.lookupScalarNode("metallic", 0.0f);
	const auto sheen		   = ctx.lookupScalarNode("sheen", 0.0f);
	const auto sheen_tint	   = ctx.lookupScalarNode("sheen_tint", 0.0f);
	const auto clearcoat	   = ctx.lookupScalarNode("clearcoat", 0.0f);
	const auto clearcoat_gloss = ctx.lookupScalarNode("clearcoat_gloss", 0.0f);

	return std::make_shared<PrincipledMaterial<UseVNDF, Thin, HasTransmission>>(
		base_color, ior, diffuse_trans, roughness, anisotropic,
		specular_trans, specular_tint, flatness, metallic, sheen,
		sheen_tint, clearcoat, clearcoat_gloss);
}

template <bool UseVNDF, bool Thin>
inline static std::shared_ptr<IMaterial> createPrincipled2(const SceneLoadContext& ctx)
{
	const bool hasTransmission = ctx.parameters().hasParameter("specular_transmission") || ctx.parameters().hasParameter("spec_trans")
								 || ctx.parameters().hasParameter("diffuse_transmission") || ctx.parameters().hasParameter("diff_trans");

	if (hasTransmission)
		return createPrincipled1<UseVNDF, Thin, true>(ctx);
	else
		return createPrincipled1<UseVNDF, Thin, false>(ctx);
}

template <bool UseVNDF>
inline static std::shared_ptr<IMaterial> createPrincipled3(const SceneLoadContext& ctx)
{
	const bool thin = ctx.parameters().getBool("thin", false);

	if (thin)
		return createPrincipled2<UseVNDF, true>(ctx);
	else
		return createPrincipled2<UseVNDF, false>(ctx);
}

class PrincipledMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		const bool use_vndf = ctx.parameters().getBool("vndf", true);

		if (use_vndf)
			return createPrincipled3<true>(ctx);
		else
			return createPrincipled3<false>(ctx);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "principled" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Principled BSDF", "Principled BSDF based on the Disney implementation")
			.Identifiers(getNames())
			.Inputs()
			.SpectralNode("base", "Base color", 0.8f)
			.SpectralNode("eta", "Index of refraction", 1.55f)
			.ScalarNode("roughness", "Roughness", 0.5f)
			.ScalarNode("anisotropic", "Amount of anisotropy", 0.0f)
			.ScalarNode("diffuse_transmission", "Diffuse transmission strength", 0.0f)
			.ScalarNode("specular_transmission", "Specular transmission strength", 0.0f)
			.ScalarNode("specular_tint", "Specular tint towards base color", 0.0f)
			.ScalarNode("flatness", "Flatness of surface. Nonzero value applies subsurface scattering", 0.0f)
			.ScalarNode("metallic", "Amount of metallic appeal", 0.0f)
			.ScalarNode("sheen", "Amount of sheen to apply", 0.0f)
			.ScalarNode("sheen_tint", "Sheen tint towards base color", 0.0f)
			.ScalarNode("clearcoat", "Amount of clearcoat to apply", 0.0f)
			.ScalarNode("clearcoat_gloss", "Glossyness of the clearcoat", 0.0f)
			.Bool("vndf", "Use sampling method based on the viewing normal", true)
			.Bool("thin", "Thin approximation", false)
			.Specification()
			.get();
	}
	
	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::PrincipledMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)