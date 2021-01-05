#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"

#include <array>
#include <sstream>

namespace PR {

enum class MaterialDelta {
	None   = 0,
	First  = 1,
	Second = 2,
	All
};

template <MaterialDelta Delta>
class AddMaterial : public IMaterial {
public:
	explicit AddMaterial(const std::shared_ptr<IMaterial>& material0, const std::shared_ptr<IMaterial>& material1)
		: IMaterial()
		, mMaterials{ material0, material1 }
	{
	}

	virtual ~AddMaterial() = default;

	MaterialFlags flags() const override
	{
		if constexpr (Delta == MaterialDelta::All)
			return MaterialFlag::OnlyDeltaDistribution;
		else
			return 0;
	}

	// TODO: Weight?
	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;

		if constexpr (Delta == MaterialDelta::All) {
			PR_ASSERT(false, "Delta distribution materials should not be evaluated");
			out.PDF_S  = 0.0f;
			out.Type   = MaterialScatteringType::SpecularTransmission;
			out.Weight = SpectralBlob::Zero();
		} else if constexpr (Delta == MaterialDelta::First) {
			mMaterials[1]->eval(in, out, session);
			out.PDF_S *= 0.5f;
		} else if constexpr (Delta == MaterialDelta::Second) {
			mMaterials[0]->eval(in, out, session);
			out.PDF_S *= 0.5f;
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

	// TODO: Weight?
	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;
		if constexpr (Delta == MaterialDelta::All) {
			PR_ASSERT(false, "Delta distribution materials should not be evaluated");
			out.PDF_S = 0.0f;
		} else if constexpr (Delta == MaterialDelta::First) {
			mMaterials[1]->pdf(in, out, session);
			out.PDF_S *= 0.5f;
		} else if constexpr (Delta == MaterialDelta::Second) {
			mMaterials[0]->pdf(in, out, session);
			out.PDF_S *= 0.5f;
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

		if (in.RND.getFloat() < PROB) {
			mMaterials[0]->sample(in, out, session);

			// We are adding them together, so no need to weight them down
			//out.Weight *= PROB;
			out.PDF_S *= PROB;
		} else {
			mMaterials[1]->sample(in, out, session);

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
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.parameters();
		const auto mat1				 = ctx.lookupMaterial(params.getParameter("material1"));
		const auto mat2				 = ctx.lookupMaterial(params.getParameter("material2"));

		if (!mat1 || !mat2) {
			PR_LOG(L_ERROR) << "Valid material1 or material2 parameters for blend material missing" << std::endl;
			return nullptr;
		}

		int deltaCount = 0;
		if (mat1->hasOnlyDeltaDistribution())
			++deltaCount;
		if (mat2->hasOnlyDeltaDistribution())
			++deltaCount;

		switch (deltaCount) {
		case 0:
		default:
			return std::make_shared<AddMaterial<MaterialDelta::None>>(mat1, mat2);
		case 1:
			if (mat1->hasOnlyDeltaDistribution())
				return std::make_shared<AddMaterial<MaterialDelta::First>>(mat1, mat2);
			else
				return std::make_shared<AddMaterial<MaterialDelta::Second>>(mat1, mat2);
		case 2:
			return std::make_shared<AddMaterial<MaterialDelta::All>>(mat1, mat2);
		}
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "add" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Add BSDF", "Add two bsdfs together")
			.Identifiers(getNames())
			.Inputs()
			.MaterialReference("material1", "First material")
			.MaterialReference("material2", "Second material")
			.Specification()
			.get();
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::AddMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)