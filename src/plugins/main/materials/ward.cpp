#include "Environment.h"
#include "Profiler.h"
#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "math/Microfacet.h"
#include "math/Projection.h"
#include "math/Reflection.h"
#include "math/Spherical.h"
#include "renderer/RenderContext.h"

#include <sstream>

namespace PR {

class WardMaterial : public IMaterial {
public:
	WardMaterial(uint32 id,
				 const std::shared_ptr<FloatSpectralShadingSocket>& alb,
				 const std::shared_ptr<FloatSpectralShadingSocket>& spec,
				 const std::shared_ptr<FloatScalarShadingSocket>& reflectivity,
				 const std::shared_ptr<FloatScalarShadingSocket>& roughnessX,
				 const std::shared_ptr<FloatScalarShadingSocket>& roughnessY)
		: IMaterial(id)
		, mAlbedo(alb)
		, mSpecularity(spec)
		, mReflectivity(reflectivity)
		, mRoughnessX(roughnessX)
		, mRoughnessY(roughnessY)
	{
	}

	virtual ~WardMaterial() = default;

	void startGroup(size_t, const RenderTileSession&) override
	{
	}

	void endGroup() override
	{
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const Vector3f H = Reflection::halfway(in.Point.Ray.Direction, in.Outgoing);

		const float NdotH = in.Point.N.dot(H);
		const float HdotX = in.Point.Nx.dot(H);
		const float HdotY = in.Point.Ny.dot(H);

		float spec = std::min(1.0f,
							  Microfacet::ward(mRoughnessX->eval(in.Point),
											   mRoughnessY->eval(in.Point),
											   -in.Point.NdotV, in.NdotL, NdotH,
											   HdotX, HdotY));

		const float refl = mReflectivity->eval(in.Point);
		out.Weight		 = mAlbedo->eval(in.Point) * (1 - refl) + spec * mSpecularity->eval(in.Point) * refl;
		out.Weight *= std::abs(in.NdotL);

		out.PDF_S = std::min(std::max(Projection::cos_hemi_pdf(in.NdotL) * (1 - refl) + spec * refl, 0.0f), 1.0f);

		out.Type = MST_DiffuseReflection;
	}

	void sampleDiffusePath(const MaterialSampleInput& in, MaterialSampleOutput& out) const
	{
		out.Outgoing = Projection::cos_hemi(in.RND[0], in.RND[1], out.PDF_S);
		out.Weight   = mAlbedo->eval(in.Point) * std::abs(out.Outgoing.dot(in.Point.N));
		out.Type	 = MST_DiffuseReflection;
	}

	void sampleSpecularPath(const MaterialSampleInput& in, MaterialSampleOutput& out) const
	{
		const float m1 = mRoughnessX->eval(in.Point);
		const float m2 = mRoughnessY->eval(in.Point);

		float cosTheta, sinTheta;			 // V samples
		float cosPhi, sinPhi;				 // U samples
		if (std::abs(m1 - m2) <= PR_EPSILON) // Isotropic
		{
			sinPhi = std::sin(2 * PR_PI * in.RND[0]);
			cosPhi = std::cos(2 * PR_PI * in.RND[0]);

			const float f = -std::log(std::max(0.00001f, in.RND[1])) * m1 * m1;
			cosTheta	  = 1 / (1 + f);
			sinTheta	  = std::sqrt(f) * cosTheta;

			const float t = 4 * PR_PI * m1 * m1 * cosTheta * cosTheta * cosTheta * in.RND[1];
			if (t <= PR_EPSILON)
				out.PDF_S = 1;
			else
				out.PDF_S = 1 / t;
		} else {
			const float pm1 = m1 * m1;
			const float pm2 = m2 * m2;

			const float f1 = (m2 / m1) * std::tan(2 * PR_PI * in.RND[0]);
			cosPhi		   = 1 / std::sqrt(1 + f1 * f1);
			sinPhi		   = f1 * cosPhi;

			const float cosPhi2 = cosPhi * cosPhi;
			const float tz		= (cosPhi2 / pm1 + sinPhi * sinPhi / pm2);
			const float f2		= -std::log(std::max(0.000001f, in.RND[1])) / tz;
			cosTheta			= 1 / (1 + f2);
			sinTheta			= std::sqrt(f2) * cosTheta;

			const float cosTheta2 = cosTheta * cosTheta;
			const float tu		  = pm1 * sinPhi * sinPhi + pm2 * cosPhi2;
			const float tb		  = 4 * PR_PI * m1 * m2 * (pm1 * (1 - cosPhi2) / cosPhi + pm2 * cosPhi) * cosTheta2;
			out.PDF_S			  = tu / tb * std::exp(-tz * (1 - cosTheta2) / (cosTheta2));
		}

		Vector3f H   = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny,
												 Spherical::cartesian(sinTheta, cosTheta, sinPhi, cosPhi));
		out.Outgoing = Reflection::reflect(H.dot(in.Point.Ray.Direction), H, in.Point.Ray.Direction);
		out.Type	 = MST_SpecularReflection;
		out.Weight   = mSpecularity->eval(in.Point) * out.PDF_S * std::abs(out.Outgoing.dot(in.Point.N));
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession&) const override
	{
		PR_PROFILE_THIS;

		const float refl = mReflectivity->eval(in.Point);

		if (in.RND[0] <= refl) {
			sampleSpecularPath(in, out);
			out.PDF_S /= refl;
		} else {
			sampleDiffusePath(in, out);
			out.PDF_S /= 1.0f - refl;
		}

		out.Outgoing = Tangent::fromTangentSpace(in.Point.N, in.Point.Nx, in.Point.Ny, out.Outgoing);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <WardMaterial>:" << std::endl
			   << "    Albedo:       " << (mAlbedo ? mAlbedo->dumpInformation() : "NONE") << std::endl
			   << "    Specularity:  " << (mSpecularity ? mSpecularity->dumpInformation() : "NONE") << std::endl
			   << "    Reflectivity: " << (mReflectivity ? mReflectivity->dumpInformation() : "NONE") << std::endl
			   << "    RoughnessX:   " << (mRoughnessX ? mRoughnessX->dumpInformation() : "NONE") << std::endl
			   << "    RoughnessY:   " << (mRoughnessY ? mRoughnessY->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

private:
	std::shared_ptr<FloatSpectralShadingSocket> mAlbedo;
	std::shared_ptr<FloatSpectralShadingSocket> mSpecularity;
	std::shared_ptr<FloatScalarShadingSocket> mReflectivity;
	std::shared_ptr<FloatScalarShadingSocket> mRoughnessX;
	std::shared_ptr<FloatScalarShadingSocket> mRoughnessY;
};

class WardMaterialFactory : public IMaterialFactory {
public:
	std::shared_ptr<IMaterial> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg = env.registry();

		const std::string albedoName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "albedo", "");

		const std::string specName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "specularity", "");

		const std::string reflName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "reflectivity", "");

		std::shared_ptr<FloatScalarShadingSocket> roughnessX;
		std::shared_ptr<FloatScalarShadingSocket> roughnessY;
		const std::string roughnessName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "roughness", "");

		if (!roughnessName.empty()) {
			roughnessX = env.getScalarShadingSocket(roughnessName, 0.5f);
			roughnessY = roughnessX;
		} else {
			const std::string roughnessXName = reg.getForObject<std::string>(
				RG_MATERIAL, uuid, "roughness_x", "");

			const std::string roughnessYName = reg.getForObject<std::string>(
				RG_MATERIAL, uuid, "roughness_y", "");

			roughnessX = env.getScalarShadingSocket(roughnessXName, 0.5f);
			roughnessY = env.getScalarShadingSocket(roughnessYName, 0.5f);
		}

		return std::make_shared<WardMaterial>(id,
											  env.getSpectralShadingSocket(albedoName, 1),
											  env.getSpectralShadingSocket(specName, 1),
											  env.getScalarShadingSocket(reflName, 1),
											  roughnessX, roughnessY);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "ward" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::WardMaterialFactory, "mat_ward", PR_PLUGIN_VERSION)