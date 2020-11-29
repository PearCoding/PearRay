#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"

#include <sstream>

namespace PR {

enum MaterialDelta {
	MD_None	  = 0,
	MD_First  = 1,
	MD_Second = 2,
	MD_All
};

template <MaterialDelta Delta>
class BlendMaterial : public IMaterial {
public:
	explicit BlendMaterial(const std::shared_ptr<IMaterial>& material0, const std::shared_ptr<IMaterial>& material1,
						   const std::shared_ptr<FloatScalarNode>& factor)
		: IMaterial()
		, mMaterials{ material0, material1 }
		, mFactor(factor)
	{
	}

	virtual ~BlendMaterial() = default;

	int flags() const override
	{
		if constexpr (Delta == MD_All)
			return MF_OnlyDeltaDistribution;
		else
			return 0;
	}

	void eval(const MaterialEvalInput& in, MaterialEvalOutput& out,
			  const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;

		if constexpr (Delta == MD_All) {
			PR_ASSERT(false, "Delta distribution materials should not be evaluated");
			out.PDF_S  = 0.0f;
			out.Type   = MST_SpecularTransmission;
			out.Weight = SpectralBlob::Zero();
		} else if constexpr (Delta == MD_First) {
			mMaterials[1]->eval(in, out, session);

			const float prob = std::min(1.0f, std::max(0.0f, mFactor->eval(in.ShadingContext)));
			out.PDF_S *= prob;
			out.Weight *= prob;
		} else if constexpr (Delta == MD_Second) {
			mMaterials[0]->eval(in, out, session);

			const float prob = std::min(1.0f, std::max(0.0f, mFactor->eval(in.ShadingContext)));
			out.PDF_S *= (1 - prob);
			out.Weight *= (1 - prob);
		} else {
			const float prob = std::min(1.0f, std::max(0.0f, mFactor->eval(in.ShadingContext)));

			MaterialEvalOutput out1;
			mMaterials[0]->eval(in, out1, session);
			MaterialEvalOutput out2;
			mMaterials[1]->eval(in, out2, session);

			out.PDF_S  = (1 - prob) * out1.PDF_S + prob * out2.PDF_S;
			out.Weight = (1 - prob) * out1.Weight + prob * out2.Weight;
			out.Type   = prob <= 0.5f ? out1.Type : out2.Type; // TODO
		}
	}

	void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out,
			 const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;
		if constexpr (Delta == MD_All) {
			PR_ASSERT(false, "Delta distribution materials should not be evaluated");
			out.PDF_S = 0.0f;
		} else if constexpr (Delta == MD_First) {
			mMaterials[1]->pdf(in, out, session);
			const float prob = std::min(1.0f, std::max(0.0f, mFactor->eval(in.ShadingContext)));
			out.PDF_S *= prob;
		} else if constexpr (Delta == MD_Second) {
			mMaterials[0]->pdf(in, out, session);
			const float prob = std::min(1.0f, std::max(0.0f, mFactor->eval(in.ShadingContext)));
			out.PDF_S *= (1 - prob);
		} else {
			const float prob = std::min(1.0f, std::max(0.0f, mFactor->eval(in.ShadingContext)));

			MaterialPDFOutput out1;
			mMaterials[0]->pdf(in, out1, session);
			MaterialPDFOutput out2;
			mMaterials[1]->pdf(in, out2, session);

			out.PDF_S = (1 - prob) * out1.PDF_S + prob * out2.PDF_S;
		}
	}

	void sample(const MaterialSampleInput& in, MaterialSampleOutput& out,
				const RenderTileSession& session) const override
	{
		PR_PROFILE_THIS;

		const float prob = std::min(1.0f, std::max(0.0f, mFactor->eval(in.ShadingContext)));

		if (in.RND[0] < (1 - prob)) {
			MaterialSampleInput in1 = in;
			in1.RND[0] /= (1 - prob);
			mMaterials[0]->sample(in1, out, session);

			out.Weight *= (1 - prob);
			out.PDF_S *= (1 - prob);
		} else {
			MaterialSampleInput in2 = in;
			in2.RND[0] /= prob;
			mMaterials[1]->sample(in2, out, session);

			out.Weight *= prob;
			out.PDF_S *= prob;
		}
	}

	std::string dumpInformation() const override
	{
		std::stringstream stream;

		stream << IMaterial::dumpInformation()
			   << "  <BlendMaterial>:" << std::endl
			   << "    [0]:         " << mMaterials[0]->dumpInformation() << std::endl
			   << "    [1]:         " << mMaterials[1]->dumpInformation() << std::endl
			   << "    Probability: " << mFactor->dumpInformation() << std::endl;

		return stream.str();
	}

private:
	const std::array<std::shared_ptr<IMaterial>, 2> mMaterials;
	const std::shared_ptr<FloatScalarNode> mFactor;
};

class BlendMaterialPlugin : public IMaterialPlugin {
public:
	std::shared_ptr<IMaterial> create(const std::string&, const SceneLoadContext& ctx)
	{
		const ParameterGroup& params = ctx.parameters();
		const auto mat1				 = ctx.lookupMaterial(params.getParameter("material1"));
		const auto mat2				 = ctx.lookupMaterial(params.getParameter("material2"));
		const auto factor			 = ctx.lookupScalarNode("factor", 0.5f);

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
			return std::make_shared<BlendMaterial<MD_None>>(mat1, mat2, factor);
		case 1:
			if (mat1->hasOnlyDeltaDistribution())
				return std::make_shared<BlendMaterial<MD_First>>(mat1, mat2, factor);
			else
				return std::make_shared<BlendMaterial<MD_Second>>(mat1, mat2, factor);
		case 2:
			return std::make_shared<BlendMaterial<MD_All>>(mat1, mat2, factor);
		}
	}

	const std::vector<std::string>& getNames() const
	{
		const static std::vector<std::string> names({ "blend", "mix" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::BlendMaterialPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)