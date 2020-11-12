#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"

#include <sstream>

namespace PR {

template <bool HasDelta>
class AddMaterial : public IMaterial {
public:
	explicit AddMaterial(uint32 id,
						 const std::shared_ptr<IMaterial>& material0, const std::shared_ptr<IMaterial>& material1)
		: IMaterial(id)
		, mMaterials{ material0, material1 }
	{
	}

	virtual ~AddMaterial() = default;

	int flags() const override
	{
		int flags = 0;
		for (const auto& mat : mMaterials)
			flags |= mat->flags();

		return flags;
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;

		if constexpr (HasDelta) {
			PR_ASSERT(false, "Delta distribution materials should not be evaluated");
			out.PDF_S  = 0.0f;
			out.Type   = MST_SpecularTransmission;
			out.Weight = SpectralBlob::Zero();
		} else {
			MaterialEvalOutput out1;
			mMaterials[0]->eval(in, out1, session);
			MaterialEvalOutput out2;
			mMaterials[1]->eval(in, out2, session);

			out.PDF_S  = (out1.PDF_S + out2.PDF_S) / 2;
			out.Weight = out1.Weight + out2.Weight;
			out.Type   = out1.Type; // TODO
		}
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;
		if constexpr (HasDelta) {
			PR_ASSERT(false, "Delta distribution materials should not be evaluated");
			out.PDF_S = 0.0f;
		} else {
			MaterialPDFOutput out1;
			mMaterials[0]->pdf(in, out1, session);
			MaterialPDFOutput out2;
			mMaterials[1]->pdf(in, out2, session);

			out.PDF_S = (out1.PDF_S + out2.PDF_S) / 2;
		}
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;
		constexpr float PROB = 0.5f;

		if (in.RND[0] < PROB) {
			MaterialSampleInput in1 = in;
			in1.RND[0] /= PROB;
			mMaterials[0]->sample(in1, out, session);

			// We are adding them together, so no need to weight them down
			//out.Weight *= PROB;
			out.PDF_S *= PROB;
		} else {
			MaterialSampleInput in2 = in;
			in2.RND[0] /= PROB;
			mMaterials[1]->sample(in2, out, session);

			//out.Weight *= PROB;
			out.PDF_S *= PROB;
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << IMaterial::dumpInformation()
			   << "  <AddMaterial>:" << std::endl
			   << "    [0]:         " << mMaterials[0]->dumpInformation() << std::endl
			   << "    [1]:         " << mMaterials[1]->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	const std::array<std::shared_ptr<IMaterial>, 2> mMaterials;
};

class AddMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(uint32 id, const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.parameters();
		const auto mat1				 = ctx.lookupMaterial(params.getParameter("material1"));
		const auto mat2				 = ctx.lookupMaterial(params.getParameter("material2"));

		if (!mat1 || !mat2) {
			PR_LOG(L_ERROR) << "Valid material1 or material2 parameters for blend material missing" << std::endl;
			return nullptr;
		}

		const bool delta = mat1->hasDeltaDistribution() || mat2->hasDeltaDistribution();
		if (delta)
			return std::make_shared<AddMaterial<true>>(id, mat1, mat2);
		else
			return std::make_shared<AddMaterial<false>>(id, mat1, mat2);
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "add" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::AddMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)