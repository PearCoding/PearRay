#include "lambert.h"
#include "Environment.h"
#include "math/Projection.h"
#include "renderer/RenderContext.h"
#include "shader/ConstShadingSocket.h"

#include "lambert_ispc.h"

#include <sstream>

namespace PR {
LambertMaterial::LambertMaterial(uint32 id)
	: IMaterial(id)
	, mAlbedo(nullptr)
{
}

std::shared_ptr<FloatSpectralShadingSocket> LambertMaterial::albedo() const
{
	return mAlbedo;
}

void LambertMaterial::setAlbedo(const std::shared_ptr<FloatSpectralShadingSocket>& diffSpec)
{
	mAlbedo = diffSpec;
}

void LambertMaterial::startGroup(size_t size, const RenderTileSession& session)
{
}

void LambertMaterial::endGroup()
{
}

void LambertMaterial::eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
						   const RenderTileSession& session) const
{
	out.Weight		   = mAlbedo->eval(in.Point);
	out.PDF_S_Forward  = simdpp::make_float(PR_1_PI);
	out.PDF_S_Backward = simdpp::make_float(PR_1_PI);
}

void LambertMaterial::sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
							 const RenderTileSession& session) const
{
	out.Weight = simdpp::make_float(0);
	out.Type   = simdpp::make_int(MST_DiffuseReflection);

	PR_SIMD_ALIGN float r0[PR_SIMD_BANDWIDTH];
	PR_SIMD_ALIGN float r1[PR_SIMD_BANDWIDTH];
	PR_SIMD_ALIGN float pf[PR_SIMD_BANDWIDTH];
	PR_SIMD_ALIGN float pb[PR_SIMD_BANDWIDTH];
	PR_SIMD_ALIGN float o0[PR_SIMD_BANDWIDTH];
	PR_SIMD_ALIGN float o1[PR_SIMD_BANDWIDTH];
	PR_SIMD_ALIGN float o2[PR_SIMD_BANDWIDTH];

	simdpp::store(r0, in.RND[0]);
	simdpp::store(r1, in.RND[1]);

	ispc::sample_lambert(r0, r1,
						 pf, pb,
						 o0, o1, o2,
						 PR_SIMD_BANDWIDTH);

	out.PDF_S_Forward  = simdpp::load(pf);
	out.PDF_S_Backward = simdpp::load(pb);
	out.Outgoing[0]	= simdpp::load(o0);
	out.Outgoing[1]	= simdpp::load(o1);
	out.Outgoing[2]	= simdpp::load(o2);
}

std::string LambertMaterial::dumpInformation() const
{
	std::stringstream stream;

	stream << std::boolalpha << IMaterial::dumpInformation()
		   << "  <DiffuseMaterial>:" << std::endl
		   << "    HasAlbedo: " << (mAlbedo ? "true" : "false") << std::endl;

	return stream.str();
}

void LambertMaterial::onFreeze(RenderContext* context)
{
}

class LambertMaterialFactory : public IMaterialFactory {
public:
	std::shared_ptr<IMaterial> create(uint32 id, uint32 uuid, const Environment& env)
	{
		return std::make_shared<LambertMaterial>(id);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "diffuse", "lambert" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::LambertMaterialFactory, "mat_lambert", PR_PLUGIN_VERSION)