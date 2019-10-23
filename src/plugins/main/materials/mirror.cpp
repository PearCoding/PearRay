#include "Environment.h"
#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "math/Projection.h"
#include "math/Reflection.h"
#include "renderer/RenderContext.h"
#include "shader/ConstShadingSocket.h"
#include "shader/ShadingSocket.h"

#include <sstream>

namespace PR {

class MirrorMaterial : public IMaterial {
public:
	MirrorMaterial(uint32 id, const std::shared_ptr<FloatSpectralShadingSocket>& alb)
		: IMaterial(id)
		, mSpecularity(alb)
	{
	}

	virtual ~MirrorMaterial() = default;

	void startGroup(size_t size, const RenderTileSession& session) override
	{
	}

	void endGroup() override
	{
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession& session) const override
	{
		out.Weight		   = mSpecularity->eval(in.Point);
		out.PDF_S_Forward  = std::numeric_limits<float>::infinity();
		out.PDF_S_Backward = std::numeric_limits<float>::infinity();
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession& session) const override
	{
		out.Weight		   = mSpecularity->eval(in.Point);
		out.Type		   = MST_SpecularReflection;
		out.PDF_S_Backward = std::numeric_limits<float>::infinity();
		out.PDF_S_Forward  = std::numeric_limits<float>::infinity();
		out.Outgoing = Reflection::reflect(in.Point.NdotV, in.Point.N, in.Point.Ray.Direction);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <MirrorMaterial>:" << std::endl
			   << "    Specularity: " << (mSpecularity ? mSpecularity->dumpInformation() : "NONE") << std::endl;

		return stream.str();
	}

protected:
	void onFreeze(RenderContext* context) override
	{
	}

private:
	std::shared_ptr<FloatSpectralShadingSocket> mSpecularity;
};

class MirrorMaterialFactory : public IMaterialFactory {
public:
	std::shared_ptr<IMaterial> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg = env.registry();

		const std::string specName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "specularity", "");

		return std::make_shared<MirrorMaterial>(id, env.getSpectralShadingSocket(specName, 1));
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "mirror", "reflection" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::MirrorMaterialFactory, "mat_mirror", PR_PLUGIN_VERSION)