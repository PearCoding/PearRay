#pragma once

#include "parameter/ParameterDesc.h"

namespace PR {
struct PluginIdentifier {
	std::string Name;
	bool AllowInline; // Note: Not all plugin types allow inlining. E.g., an integrator plugin is ignoring this property
};

enum class PluginParamDescBlockOp {
	And = 0,
	Or,
	OneOf,
	Optional
};

// It would be possible to remove the explicit heap allocation with recursive variants
struct PluginParamDescBlock {
	using Variant = std::variant<ParameterDesc, std::shared_ptr<PluginParamDescBlock>>;
	std::string Name;
	PluginParamDescBlockOp Operator = PluginParamDescBlockOp::And;

	std::vector<Variant> Inputs;
};

struct PluginSpecification {
public:
	std::string Name		= nullptr;
	std::string Description = nullptr;
	std::vector<PluginIdentifier> Identifiers;
	PluginParamDescBlock Inputs;

	inline PluginSpecification(const std::string& name, const std::string& description)
		: Name(name)
		, Description(description)
	{
	}

	inline bool allowsInline() const
	{
		for (auto& n : Identifiers) {
			if (!n.AllowInline)
				return false;
		}
		return true;
	}
};

//////////////////////// Builder
class PluginSpecificationBuilder;

class PluginParamDescBlockBuilder {
private:
	inline static ParameterFlags makeOptional(bool optional)
	{
		if (optional)
			return ParameterFlag::Optional;
		else
			return 0;
	}

	PluginSpecificationBuilder* mTop;
	PluginParamDescBlockBuilder* mParent;
	PluginParamDescBlock& mBlock;

public:
	inline PluginParamDescBlockBuilder(PluginSpecificationBuilder* top, PluginParamDescBlockBuilder* parent, PluginParamDescBlock& block)
		: mTop(top)
		, mParent(parent)
		, mBlock(block)
	{
	}

	inline PluginParamDescBlockBuilder& Bool(const std::string& name, const std::string& desc, bool def, bool optional = false)
	{
		return BoolV({ name }, desc, def, optional);
	}

	inline PluginParamDescBlockBuilder& BoolV(const std::vector<std::string>& names, const std::string& desc, bool def, bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterBoolDesc{ names, desc, def, makeOptional(optional) }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& Int(const std::string& name, const std::string& desc, Parameter::Int def,
											Parameter::Int soft_min = std::numeric_limits<Parameter::Int>::min(), Parameter::Int soft_max = std::numeric_limits<Parameter::Int>::max(),
											Parameter::Int min = std::numeric_limits<Parameter::Int>::min(), Parameter::Int max = std::numeric_limits<Parameter::Int>::max(),
											bool optional = false)
	{
		return IntV({ name }, desc, def, soft_min, soft_max, min, max, optional);
	}

	inline PluginParamDescBlockBuilder& IntV(const std::vector<std::string>& names, const std::string& desc, Parameter::Int def,
											Parameter::Int soft_min = std::numeric_limits<Parameter::Int>::min(), Parameter::Int soft_max = std::numeric_limits<Parameter::Int>::max(),
											Parameter::Int min = std::numeric_limits<Parameter::Int>::min(), Parameter::Int max = std::numeric_limits<Parameter::Int>::max(),
											bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterIntDesc{ { names, desc, def, makeOptional(optional) }, soft_min, soft_max, min, max }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& UInt(const std::string& name, const std::string& desc, Parameter::UInt def,
											 Parameter::UInt soft_min = std::numeric_limits<Parameter::UInt>::min(), Parameter::UInt soft_max = std::numeric_limits<Parameter::UInt>::max(),
											 Parameter::UInt min = std::numeric_limits<Parameter::UInt>::min(), Parameter::UInt max = std::numeric_limits<Parameter::UInt>::max(),
											 bool optional = false)
	{
		return UIntV({ name }, desc, def, soft_min, soft_max, min, max, optional);
	}

	inline PluginParamDescBlockBuilder& UIntV(const std::vector<std::string>& names, const std::string& desc, Parameter::UInt def,
											 Parameter::UInt soft_min = std::numeric_limits<Parameter::UInt>::min(), Parameter::UInt soft_max = std::numeric_limits<Parameter::UInt>::max(),
											 Parameter::UInt min = std::numeric_limits<Parameter::UInt>::min(), Parameter::UInt max = std::numeric_limits<Parameter::UInt>::max(),
											 bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterUIntDesc{ { names, desc, def, makeOptional(optional) }, soft_min, soft_max, min, max }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& Number(const std::string& name, const std::string& desc, Parameter::Number def,
											   Parameter::Number soft_min = std::numeric_limits<Parameter::Number>::min(), Parameter::Number soft_max = std::numeric_limits<Parameter::Number>::max(),
											   Parameter::Number min = std::numeric_limits<Parameter::Number>::min(), Parameter::Number max = std::numeric_limits<Parameter::Number>::max(),
											   bool optional = false)
	{
		return NumberV({ name }, desc, def, soft_min, soft_max, min, max, optional);
	}

	inline PluginParamDescBlockBuilder& NumberV(const std::vector<std::string>& names, const std::string& desc, Parameter::Number def,
											   Parameter::Number soft_min = std::numeric_limits<Parameter::Number>::min(), Parameter::Number soft_max = std::numeric_limits<Parameter::Number>::max(),
											   Parameter::Number min = std::numeric_limits<Parameter::Number>::min(), Parameter::Number max = std::numeric_limits<Parameter::Number>::max(),
											   bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterNumberDesc{ { names, desc, def, makeOptional(optional) }, soft_min, soft_max, min, max }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& Number01(const std::string& name, const std::string& desc, Parameter::Number def, bool optional = false)
	{
		return Number01V({ name }, desc, def, optional);
	}

	inline PluginParamDescBlockBuilder& Number01V(const std::vector<std::string>& names, const std::string& desc, Parameter::Number def, bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterNumberDesc{ { names, desc, def, makeOptional(optional) }, 0, 1, 0, 1 }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& Vector(const std::string& name, const std::string& desc, const Vector3f& def, bool optional = false)
	{
		return VectorV({ name }, desc, def, optional);
	}

	inline PluginParamDescBlockBuilder& VectorV(const std::vector<std::string>& names, const std::string& desc, const Vector3f& def, bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterVectorDesc{ { names, desc, def, makeOptional(optional) } }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& String(const std::string& name, const std::string& desc, const std::string& def, bool optional = false)
	{
		return StringV({ name }, desc, def, optional);
	}

	inline PluginParamDescBlockBuilder& StringV(const std::vector<std::string>& names, const std::string& desc, const std::string& def, bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterStringDesc{ { names, desc, def, makeOptional(optional) } }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& Option(const std::string& name, const std::string& desc, const std::string& def, const std::vector<std::string>& options, bool optional = false)
	{
		return OptionV({ name }, desc, def, options, optional);
	}

	inline PluginParamDescBlockBuilder& OptionV(const std::vector<std::string>& names, const std::string& desc, const std::string& def, const std::vector<std::string>& options, bool optional = false)
	{
		PR_ASSERT(options.size() > 0, "Expected at least one option to choose from");

		if (def.empty()) {
			mBlock.Inputs.emplace_back(ParameterDesc(ParameterOptionDesc{ { names, desc, options.front(), makeOptional(optional) }, options }));
		} else {
			PR_ASSERT(std::find(options.begin(), options.end(), def) != options.end(), "Given default value should be one of the given options!");
			mBlock.Inputs.emplace_back(ParameterDesc(ParameterOptionDesc{ { names, desc, def, makeOptional(optional) }, options }));
		}
		return *this;
	}

	inline PluginParamDescBlockBuilder& Filename(const std::string& name, const std::string& desc, bool optional = false)
	{
		return FilenameV({ name }, desc, optional);
	}

	inline PluginParamDescBlockBuilder& FilenameV(const std::vector<std::string>& names, const std::string& desc, bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterFilenameDesc{ { names, desc, "", makeOptional(optional) | ParameterFlag::NoDefault } }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& ScalarNode(const std::string& name, const std::string& desc, float def, bool optional = false)
	{
		return ScalarNodeV({ name }, desc, def, optional);
	}

	inline PluginParamDescBlockBuilder& ScalarNodeV(const std::vector<std::string>& names, const std::string& desc, float def, bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterScalarNodeDesc{ { names, desc, def, makeOptional(optional) } }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& SpectralNode(const std::string& name, const std::string& desc, float def, bool optional = false)
	{
		return SpectralNodeV({ name }, desc, def, optional);
	}

	inline PluginParamDescBlockBuilder& SpectralNodeV(const std::vector<std::string>& names, const std::string& desc, float def, bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterSpectralNodeDesc{ { names, desc, def, makeOptional(optional) } }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& VectorNode(const std::string& name, const std::string& desc, const Vector3f& def, bool optional = false)
	{
		return VectorNodeV({ name }, desc, def, optional);
	}

	inline PluginParamDescBlockBuilder& VectorNodeV(const std::vector<std::string>& names, const std::string& desc, const Vector3f& def, bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterVectorNodeDesc{ { names, desc, def, makeOptional(optional) } }));
		return *this;
	}

	inline PluginParamDescBlockBuilder& MeshReference(const std::string& name, const std::string& desc, bool optional = false)
	{
		return MeshReferenceV({ name }, desc, optional);
	}

	inline PluginParamDescBlockBuilder& MeshReferenceV(const std::vector<std::string>& names, const std::string& desc, bool optional = false)
	{
		return ReferenceV(ParameterReferenceType::Mesh, names, desc, optional);
	}

	inline PluginParamDescBlockBuilder& MaterialReference(const std::string& name, const std::string& desc, bool optional = false)
	{
		return MaterialReferenceV({ name }, desc, optional);
	}

	inline PluginParamDescBlockBuilder& MaterialReferenceV(const std::vector<std::string>& names, const std::string& desc, bool optional = false)
	{
		return ReferenceV(ParameterReferenceType::Material, names, desc, optional);
	}

	inline PluginParamDescBlockBuilder& EmissionReference(const std::string& name, const std::string& desc, bool optional = false)
	{
		return MaterialReferenceV({ name }, desc, optional);
	}

	inline PluginParamDescBlockBuilder& EmissionReferenceV(const std::vector<std::string>& names, const std::string& desc, bool optional = false)
	{
		return ReferenceV(ParameterReferenceType::Emission, names, desc, optional);
	}

	inline PluginParamDescBlockBuilder& UnknownReference(const std::string& name, const std::string& desc, bool optional = false)
	{
		return UnknownReferenceV({ name }, desc, optional);
	}

	inline PluginParamDescBlockBuilder& UnknownReferenceV(const std::vector<std::string>& names, const std::string& desc, bool optional = false)
	{
		return ReferenceV(ParameterReferenceType::Unknown, names, desc, optional);
	}

	inline PluginParamDescBlockBuilder& Reference(ParameterReferenceType type, const std::string& name, const std::string& desc, bool optional = false)
	{
		return ReferenceV(type, { name }, desc, optional);
	}

	inline PluginParamDescBlockBuilder& ReferenceV(ParameterReferenceType type, const std::vector<std::string>& names, const std::string& desc, bool optional = false)
	{
		mBlock.Inputs.emplace_back(ParameterDesc(ParameterReferenceDesc{ { names, desc, 0, makeOptional(optional) | ParameterFlag::NoDefault }, type }));
		return *this;
	}

	inline PluginParamDescBlockBuilder BeginBlock(const std::string& name, PluginParamDescBlockOp op = PluginParamDescBlockOp::And)
	{
		auto block		= std::make_shared<PluginParamDescBlock>();
		block->Name		= name;
		block->Operator = op;

		mBlock.Inputs.emplace_back(block);
		return PluginParamDescBlockBuilder(mTop, this, *block);
	}

	inline PluginParamDescBlockBuilder EndBlock()
	{
		PR_ASSERT(mParent, "Can not go back from a block if it is the top block!");
		return PluginParamDescBlockBuilder(mTop, mParent->mParent, mParent->mBlock);
	}

	inline PluginSpecificationBuilder& Specification()
	{
		PR_ASSERT(!mParent, "Should not go back to the specification if it is not the top block!");
		PR_ASSERT(mTop, "Expected top block to have a specification associated with it!");
		return *mTop;
	}
};

class PluginSpecificationBuilder {
public:
	PluginSpecification Specification;

	inline PluginSpecificationBuilder(const std::string& name, const std::string& desc)
		: Specification(name, desc)
	{
	}

	inline PluginSpecification get() const { return Specification; }

	inline PluginSpecificationBuilder& Identifier(const std::string& name, bool allowInline = true)
	{
		Specification.Identifiers.emplace_back(PluginIdentifier{ name, allowInline });
		return *this;
	}

	inline PluginSpecificationBuilder& Identifiers(const std::vector<std::string>& names, bool allowInline = true)
	{
		for (auto& name : names)
			Specification.Identifiers.emplace_back(PluginIdentifier{ name, allowInline });
		return *this;
	}

	inline PluginParamDescBlockBuilder Inputs()
	{
		return PluginParamDescBlockBuilder(this, nullptr, Specification.Inputs);
	}
};
} // namespace PR