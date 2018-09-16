#include "lambert.h"
#include "ray/Ray.h"
#include "renderer/RenderContext.h"
#include "shader/ConstSpectralOutput.h"
#include "shader/ShaderClosure.h"

#include "math/Projection.h"

#include "lambert_ispc.h"

#include <sstream>

namespace PR {
LambertMaterial::LambertMaterial(uint32 id)
	: IMaterial(id)
	, mAlbedo(nullptr)
{
}

std::shared_ptr<SpectrumShaderOutput> LambertMaterial::albedo() const
{
	return mAlbedo;
}

void LambertMaterial::setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec)
{
	mAlbedo = diffSpec;
}

void LambertMaterial::startGroup(size_t size, const RenderSession& session)
{
}

void LambertMaterial::endGroup()
{
}

void LambertMaterial::eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
						   const RenderSession& session) const
{
	PR_ASSERT(in.Size <= MATERIAL_GROUP_SIZE, "Bad size given");
	out.Size = in.Size;

	for (size_t i = 0; i < in.Size; ++i) {
		out.Weight[i]		  = 0;		 // TODO
		out.PDF_S_Forward[i]  = PR_1_PI; // TODO: Base it on the normal
		out.PDF_S_Backward[i] = PR_1_PI;
	}
}

void LambertMaterial::sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
							 const RenderSession& session) const
{
	PR_ASSERT(in.Size <= MATERIAL_GROUP_SIZE, "Bad size given");
	out.Size = in.Size;
	for (size_t i = 0; i < in.Size; ++i) {
		out.Weight[i] = 0; // TODO
		out.Type[i]   = MST_DiffuseReflection;
	}

	ispc::sample_lambert(in.RND[0].data(), in.RND[1].data(),
		out.PDF_S_Forward.data(), out.PDF_S_Backward.data(),
		out.Outgoing[0].data(),out.Outgoing[1].data(),out.Outgoing[2].data(),
		in.Size);
}

std::string LambertMaterial::dumpInformation() const
{
	std::stringstream stream;

	stream << std::boolalpha << IMaterial::dumpInformation()
		   << "  <DiffuseMaterial>:" << std::endl
		   << "    HasAlbedo: " << (mAlbedo ? "true" : "false") << std::endl;

	return stream.str();
}

class LambertMaterialFactory : public IMaterialFactory {
public:
	std::shared_ptr<IMaterial> create(uint32 id, uint32 uuid, const Registry& reg)
	{
		return std::make_shared<LambertMaterial>(id);
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "diffuse", "lambert" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::LambertMaterialFactory, "lambert", "1.0")