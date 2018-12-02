#include "emission/IEmission.h"
#include "emission/IEmissionFactory.h"

#include "math/Projection.h"
#include "registry/Registry.h"
#include "shader/ConstShadingSocket.h"

namespace PR {
class DiffuseEmission : public IEmission {
public:
	DiffuseEmission(uint32 id)
		: IEmission(id)
	{
	}

	/*virtual void startGroup(size_t size, const RenderTileSession& session) override {
	}

	virtual void endGroup() override {
	}*/

	void setRadiance(const std::shared_ptr<FloatSpectralShadingSocket>& spec)
	{
		mRadiance = spec;
	}

	void eval(const LightEvalInput& in, LightEvalOutput& out, const RenderTileSession& session) const override
	{
		out.Weight = mRadiance->eval(in.Point);
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << std::boolalpha << IEmission::dumpInformation()
			   << "  <DiffuseEmission>:" << std::endl
			   << "    HasRadiance: " << (mRadiance ? "true" : "false") << std::endl;

		return stream.str();
	}

	void onFreeze(RenderContext* context) override
	{
	}

private:
	std::shared_ptr<FloatSpectralShadingSocket> mRadiance;
};

class DiffuseEmissionFactory : public IEmissionFactory {
public:
	std::shared_ptr<IEmission> create(uint32 id, uint32 uuid, const Registry& reg) override
	{
		// TODO
		return std::make_shared<DiffuseEmission>(id);
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "diffuse", "standard", "default" });
		return names;
	}

	bool init() override
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::DiffuseEmissionFactory, "ems_diffuse", PR_PLUGIN_VERSION)