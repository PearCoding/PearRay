#include "Environment.h"
#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "math/Projection.h"
#include "renderer/RenderContext.h"
#include "shader/ConstShadingSocket.h"
#include "shader/ShadingSocket.h"

#include <sstream>

namespace PR {

class GlassMaterial : public IMaterial {
public:
	GlassMaterial(uint32 id, const std::shared_ptr<FloatSpectralShadingSocket>& alb)
		: IMaterial(id)
		, mSpecularity(alb)
	{
	}

	virtual ~GlassMaterial() = default;

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
		out.Type		   = MST_SpecularReflection; // For now
		out.PDF_S_Backward = std::numeric_limits<float>::infinity();
		out.PDF_S_Forward  = std::numeric_limits<float>::infinity();
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IMaterial::dumpInformation()
			   << "  <GlassMaterial>:" << std::endl;

		return stream.str();
	}

protected:
	void onFreeze(RenderContext* context) override
	{
	}

private:
	std::shared_ptr<FloatSpectralShadingSocket> mSpecularity;
};

class GlassMaterialFactory : public IMaterialFactory {
public:
	std::shared_ptr<IMaterial> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg = env.registry();

		const std::string specName = reg.getForObject<std::string>(
			RG_MATERIAL, uuid, "specularity", "");

		return std::make_shared<GlassMaterial>(id, env.getSpectralShadingSocket(specName, 1));
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "glass" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::GlassMaterialFactory, "mat_glass", PR_PLUGIN_VERSION)